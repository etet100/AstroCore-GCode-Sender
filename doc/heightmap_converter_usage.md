# ApplyHeightmap Converter - Usage Guide

## Overview

`ApplyHeightmap` is a G-code converter that applies surface height compensation to movement commands. It automatically segments long movements into shorter segments and adjusts Z coordinates based on heightmap data.

## How It Works

1. **Segmentation**: Long movement lines (G0/G1) are divided into segments (default 1mm)
2. **Height Interpolation**: For each segment endpoint, Z offset is interpolated from heightmap
3. **Z Adjustment**: The interpolated offset is added to Z coordinate
4. **Arc Handling**: Arcs (G2/G3) are automatically expanded and processed
5. **State Tracking**: Uses `GcodeParser` to track G90/G91, G20/G21, and other modes

## Basic Usage

```cpp
#include "core/gcode/converter/applyheightmap.h"

// Create heightmap (loaded from file or created from probe data)
Heightmap* heightmap = loadHeightmapFromFile("surface.map");

// Create converter with 1mm segment length
ApplyHeightmap converter(heightmap, 1.0);

// Set source G-code
converter.setGCode(originalGCode);

// Convert all at once (RECOMMENDED)
GCode* result = converter.convertAll();

// OR: Pull mode - convert in batches
while (converter.hasMore()) {
    converter.convertNext(100);  // Process 100 lines
}
```

**IMPORTANT:** `ApplyHeightmap` implements `ConverterInterface` directly and should
**NOT** be used in a `Pipeline` with other converters. It creates new lines during
processing which would cause issues with the Pipeline architecture.

## Parameters

### Segment Length
Controls maximum distance between heightmap samples:

```cpp
// Fine segmentation (0.5mm) - more accurate but more G-code lines
ApplyHeightmap converter(heightmap, 0.5);

// Normal segmentation (1.0mm) - balanced
ApplyHeightmap converter(heightmap, 1.0);

// Coarse segmentation (2.0mm) - faster but less accurate
ApplyHeightmap converter(heightmap, 2.0);

// Change after creation
converter.setSegmentLength(0.5);
```

**Recommendations:**
- **Fine detail work**: 0.3-0.5mm
- **General machining**: 0.8-1.2mm
- **Rough passes**: 1.5-2.5mm

### Interpolation Mode

Interpolation mode is inherited from the Heightmap object:

```cpp
heightmap->setInterpolationMode(Heightmap::InterpolationMode::Bicubic);
// Bicubic (default) - smoothest, best for curved surfaces
// Bilinear - good balance of speed and accuracy
// Linear - fastest, sufficient for flat areas
```

## Example: Complete Workflow

```cpp
// 1. Load G-code
GCode* originalGCode = loadGCodeFromFile("part.nc");

// 2. Load heightmap from probe data
Heightmap* heightmap = loadHeightmapFromFile("table_probe.map");

// 3. Pre-process G-code if needed (optional)
// Use Pipeline for modifications that don't insert lines
Pipeline prepPipeline;
prepPipeline << new FeedRateConverter(1.5);
prepPipeline << new CoordinateOffsetConverter(10, 10, 0);
prepPipeline.setGCode(originalGCode);
GCode* preprocessedGCode = prepPipeline.convertAll();

// 4. Apply heightmap (standalone, not in pipeline)
ApplyHeightmap heightmapConverter(heightmap, 1.0);
heightmapConverter.setGCode(preprocessedGCode);

// Show progress
connect(&heightmapConverter, &ApplyHeightmap::progressChanged,
    [](int current, int total) {
        qDebug() << "Progress:" << (current * 100 / total) << "%";
    });

// Convert all lines
GCode* compensatedGCode = heightmapConverter.convertAll();

// 5. Save result
saveGCodeToFile(compensatedGCode, "part_compensated.nc");
```

## What Gets Modified

### Linear Movements (G0/G1)
**Before:**
```gcode
G1 X10.0 Y10.0 Z-2.0 F500
```

**After (if distance > segment length):**
```gcode
G1 X1.111 Y1.111 Z-1.897 F500
G1 X2.222 Y2.222 Z-1.904
G1 X3.333 Y3.333 Z-1.909
...
G1 X10.0 Y10.0 Z-1.985
```

### Arcs (G2/G3)
Arcs are automatically expanded into line segments using existing utilities:

**Before:**
```gcode
G2 X10.0 Y0.0 I5.0 J0.0 F500
```

**After:**
```gcode
G1 X0.524 Y0.524 Z-1.957 F500
G1 X1.043 Y1.043 Z-1.961
G1 X1.556 Y1.556 Z-1.967
...
G1 X10.0 Y0.0 Z-1.985
```

### Preserved Elements
- Feed rates (F) on first segment only
- Comments are preserved
- Non-movement commands (M, S, etc.) pass through unchanged
- G-code modal state (G90/G91, G20/G21) is tracked correctly

## Points Outside Heightmap

If a point falls outside the heightmap area:
- No Z offset is applied
- Original Z coordinate is preserved
- Processing continues normally

Check with:
```cpp
if (!heightmap->isInside(QPointF(x, y))) {
    qWarning() << "Point outside heightmap area:" << x << y;
}
```

## Performance Considerations

### Processing Time
Typical processing time depends on:
- Number of movement lines
- Segment length (shorter = more lines)
- Original path complexity

**Example:** 10,000 line G-code file with 1mm segments:
- ~0.5-2 seconds on modern CPU
- Result may have 50,000-100,000 lines

### Memory Usage
- Each segmented line creates new `GCodeItem`
- Estimate: ~200 bytes per line
- 100,000 lines ≈ 20 MB

### Optimization Tips
1. **Pre-filter G-code**: Remove comment-only lines before conversion
2. **Adjust segment length**: Use coarser segments where acceptable
3. **Process in batches**: Use `convertNext(count)` for incremental processing
4. **Simplify arcs**: Set appropriate arc precision in heightmap

## Troubleshooting

### Issue: Segments are uneven or jagged
**Cause:** Heightmap has noisy data or insufficient resolution
**Solution:**
- Increase probe density when creating heightmap
- Use Bicubic interpolation for smoother results
- Apply smoothing filter to heightmap data

### Issue: Z coordinates seem incorrect
**Cause:** G90/G91 mode mismatch or units issue
**Solution:**
- Ensure G-code uses G90 (absolute mode) for Z
- Verify heightmap and G-code use same units (mm)
- Check that heightmap area matches G-code working area

### Issue: Processing is very slow
**Cause:** Too fine segmentation or very long paths
**Solution:**
- Increase segment length (e.g., from 0.5mm to 1.5mm)
- Pre-process to convert long rapids (G0) to direct moves

### Issue: Arc movements look wrong
**Cause:** Arc expansion conflicts with heightmap
**Solution:**
- Use separate ArcsToLines converter before ApplyHeightmap
- Adjust arc precision to match segment length

## Integration with UI

Example Qt integration:

```cpp
// In main window or controller
void MainWindow::applyHeightmapToProgram()
{
    if (!m_heightmap || !m_gcode) {
        return;
    }

    // Create progress dialog
    QProgressDialog progress("Applying heightmap...", "Cancel", 0, 100, this);
    progress.setWindowModality(Qt::WindowModal);

    // Setup converter
    ApplyHeightmap converter(m_heightmap,
        ui->spinSegmentLength->value());

    SingleConverter wrapper(&converter);
    wrapper.setGCode(m_gcode);

    // Connect progress
    connect(&wrapper, &SingleConverter::progressChanged,
        [&](int current, int total) {
            progress.setValue(current * 100 / total);
        });

    // Process
    GCode* result = wrapper.convertAll();

    if (result && !progress.wasCanceled()) {
        // Replace original with compensated version
        m_gcode = result;
        updateVisualization();
        statusBar()->showMessage("Heightmap applied successfully");
    }
}
```

## Algorithm Details

### Segmentation Strategy

For a line from point A to point B:
1. Calculate distance: `d = |B - A|`
2. Calculate segments needed: `n = ceil(d / segmentLength)`
3. Create n+1 points along line (including A and B)
4. For each point, interpolate Z offset from heightmap
5. Generate G-code line for each segment

### Coordinate Systems

The converter handles:
- **G90 (Absolute)**: X/Y/Z are absolute coordinates
- **G91 (Incremental)**: X/Y/Z are relative to current position
- **G92 (Offset)**: Tracked by parser, no special handling needed
- **G20/G21 (Units)**: Tracked by parser, assumes heightmap is in mm

### Z Offset Calculation

```
finalZ = originalZ + heightmapInterpolation(x, y)
```

Where:
- `originalZ`: Z coordinate from original G-code
- `heightmapInterpolation(x, y)`: Interpolated height at XY position
- `finalZ`: Compensated Z coordinate

## See Also

- [Heightmap Format Specification](../readme.md#heightmap)
- [Probing Behavior Documentation](probing_behavior_usage.md)
- [Converter Architecture](../src/astrocore/core/gcode/converter/README.md)

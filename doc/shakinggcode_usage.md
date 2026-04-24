# ShakingGCode Converter - Test Module

## Overview

`ShakingGCode` is a test/demonstration converter that intentionally distorts G-code paths by:
- Segmenting movement lines every 5mm (configurable)
- Adding random XYZ offset to each segment endpoint (±1mm configurable)
- Randomly varying feed rates (±20% configurable)

**Purpose:** Testing, demonstration, stress testing visualizers, creating intentional path distortion.

## Basic Usage

```cpp
#include "core/gcode/converter/shakinggcode.h"

// Create shaker with default settings (5mm segments, ±1mm offset)
ShakingGCode shaker;
shaker.setGCode(originalGCode);

// Convert all at once
GCode* shakenGCode = shaker.convertAll();
```

## Configuration Options

### Segment Length
Distance between segment points:

```cpp
ShakingGCode shaker(5.0, 1.0);  // 5mm segments
shaker.setSegmentLength(2.0);    // Change to 2mm segments
```

### Random Offset Range
Maximum random offset applied to each axis:

```cpp
ShakingGCode shaker(5.0, 1.0);  // ±1mm offset
shaker.setMaxOffset(0.5);        // Change to ±0.5mm
shaker.setMaxOffset(2.0);        // Change to ±2mm
```

### Feed Rate Variation
Random feed rate modification (0.0 = no variation, 1.0 = ±100%):

```cpp
ShakingGCode shaker;
shaker.setFeedRateVariation(0.2);  // ±20% (default)
shaker.setFeedRateVariation(0.1);  // ±10% (subtle)
shaker.setFeedRateVariation(0.5);  // ±50% (extreme)
```

### Random Seed
For reproducible results:

```cpp
ShakingGCode shaker;
shaker.setSeed(42);  // Same seed = same random sequence
```

## Complete Example

```cpp
// Load original G-code
GCode* originalGCode = loadGCodeFromFile("test.nc");

// Configure shaker
ShakingGCode shaker(
    3.0,   // Segment every 3mm
    0.8    // ±0.8mm random offset
);
shaker.setFeedRateVariation(0.15);  // ±15% feed rate
shaker.setSeed(12345);              // Reproducible randomness

// Connect progress
QObject::connect(&shaker, &ShakingGCode::progressChanged,
    [](int current, int total) {
        qDebug() << "Shaking progress:" << (current * 100 / total) << "%";
    });

// Convert
shaker.setGCode(originalGCode);
GCode* result = shaker.convertAll();

// Save
saveGCodeToFile(result, "test_shaken.nc");
```

## Example Transformations

### Original Line
```gcode
G1 X100.0 Y50.0 Z-2.0 F1000
```

### After ShakingGCode (segment=5mm, offset=±1mm, feedVar=0.2)
```gcode
G1 X5.124 Y2.487 Z-2.156 F983.2
G1 X10.089 Y4.912 Z-1.847 F1042.7
G1 X14.967 Y7.543 Z-2.234 F989.5
G1 X19.854 Y9.876 Z-1.892 F1098.3
...
G1 X99.876 Y49.812 Z-2.087 F976.8
```

## Use Cases

### 1. Testing Visualizers
Verify that your 3D visualizer handles dense, irregular paths:

```cpp
ShakingGCode shaker(1.0, 0.5);  // Very dense
shaker.setGCode(smoothPath);
GCode* zigzagPath = shaker.convertAll();
// Display in visualizer to test rendering performance
```

### 2. Simulating Machine Vibration
Generate G-code that looks like output from a shaky machine:

```cpp
ShakingGCode shaker(10.0, 0.1);  // Small shake
shaker.setFeedRateVariation(0.05);
// Results look like real machine inaccuracy
```

### 3. Creating Test Patterns
Generate complex paths for algorithm testing:

```cpp
// Start with simple square
GCode* square = createSquareGCode(50, 50);

// Add complexity
ShakingGCode shaker(2.0, 0.3);
GCode* complexSquare = shaker.convertAll();

// Test path optimization algorithms on complex paths
```

### 4. Feed Rate Testing
Test how controller handles varying feed rates:

```cpp
ShakingGCode shaker(5.0, 0.0);  // No position shake
shaker.setFeedRateVariation(0.3);  // Only vary feed rate
// Every segment has different speed
```

## Implementation Notes

### What Gets Modified
- **Linear movements (G0/G1)**: Segmented and offset
- **Arcs (G2/G3)**: Pass through unchanged (simplicity)
- **Non-movement commands**: Pass through unchanged
- **Comments**: Preserved

### Coordinates
All coordinates (X, Y, Z) are explicitly written in output for clarity.

### Feed Rates
Feed rate is written on every segment to make each independently controllable.

### Parser State
Internal `GcodeParser` tracks state (G90/G91, G20/G21) correctly.

## Performance

**Typical processing:**
- Input: 1,000 lines
- Output: ~20,000 lines (with 5mm segments on long moves)
- Time: ~100-300ms on modern CPU

**Memory:**
- ~200 bytes per output line
- 20,000 lines ≈ 4 MB

## Combining with Other Converters

### Pre-processing
Apply other modifications first:

```cpp
// Step 1: Offset coordinates
Pipeline prep;
prep << new CoordinateOffsetConverter(10, 10, 0);
prep.setGCode(originalGCode);
GCode* offsetGCode = prep.convertAll();

// Step 2: Shake
ShakingGCode shaker;
shaker.setGCode(offsetGCode);
GCode* result = shaker.convertAll();
```

### Post-processing
Not recommended - shaken paths are already complex.

## Warnings

⚠️ **DO NOT use on real machining jobs!** This converter intentionally makes paths inaccurate.

⚠️ **Large files**: Shaking creates many lines. A 10,000 line program might become 200,000+ lines.

⚠️ **Z-axis**: Random Z offsets can be dangerous in real machining. Use `maxOffset = 0` to disable Z:

```cpp
// Custom: only shake X and Y
void applyRandomOffset(QVector3D &point) {
    double offsetX = (m_random->generateDouble() * 2.0 - 1.0) * m_maxOffset;
    double offsetY = (m_random->generateDouble() * 2.0 - 1.0) * m_maxOffset;
    // No Z offset for safety
    point.setX(point.x() + offsetX);
    point.setY(point.y() + offsetY);
}
```

## Comparison with ApplyHeightmap

| Feature | ShakingGCode | ApplyHeightmap |
|---------|--------------|----------------|
| Purpose | Testing/Demo | Real compensation |
| Offset source | Random | Heightmap interpolation |
| Z modification | All axes | Z only |
| Feed rate | Varies randomly | Preserves original |
| Use in production | ❌ Never | ✅ Yes |

## Future Enhancements

Ideas for extending this test module:

1. **Sine wave offset**: Instead of random, apply sinusoidal pattern
2. **Arc support**: Segment and shake arcs too
3. **Selective axes**: Choose which axes to shake (X/Y/Z independently)
4. **Offset profiles**: Different offset patterns (linear ramp, exponential, etc.)
5. **Feed rate profiles**: Systematic speed variations

## Example: Visualize in G-Pilot

```cpp
// In main window
void MainWindow::onShakeGCodeClicked() {
    if (!m_gcode) return;

    // Get parameters from UI
    double segmentLength = ui->spinShakeSegment->value();
    double maxOffset = ui->spinShakeOffset->value();

    // Create shaker
    ShakingGCode shaker(segmentLength, maxOffset);
    shaker.setGCode(m_gcode);

    // Show progress
    QProgressDialog progress("Shaking G-code...", "Cancel", 0, 100, this);
    connect(&shaker, &ShakingGCode::progressChanged,
        [&](int curr, int total) {
            progress.setValue(curr * 100 / total);
        });

    // Convert
    GCode* result = shaker.convertAll();

    if (result && !progress.wasCanceled()) {
        m_gcode = result;
        updateVisualization();
        statusBar()->showMessage(
            QString("Shaken! %1 → %2 lines")
                .arg(m_gcode->count())
                .arg(result->count())
        );
    }
}
```

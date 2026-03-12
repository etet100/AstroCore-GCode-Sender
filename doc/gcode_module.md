# G-Code Module Documentation

## Overview

The G-code module lives in `src/gpilot/core/gcode/`. It handles loading, parsing,
rendering, and conversion of G-code files.

---

## Key data structures

### GCodeItem  (`gcode/gcode.h`)
Raw parsed record of one G-code line. Stored in the `GCode` list.

| Field | Type | Description |
|---|---|---|
| `line` | QString | Full text of the line (modified by converters) |
| `command` | QString | First G/M command, e.g. "G1", "M3" |
| `args` | QStringList | Tokenised arguments, e.g. ["G1","X10.0","Y20.0","F300"] |
| `comment` | QString | Text extracted from `(...)` or `;` comment |
| `commandNumber` | int | Sequential index assigned by GcodeParser |
| `state` | States | InQueue / Sent / Processed / Error / Skipped / Comment |
| `group` | GCodeItemGroup | Movement / ArcMovement / Dwell / Spindle / ... |
| `isMovement` | bool | True if parser generated a PointSegment for this line |
| `isArc()` | method | True if command == "G2" or "G3" |

### PointSegment  (`gcode/parser/pointsegment.h`)
One endpoint produced by GcodeParser. Carries the full machine state at that point.

Fields: `point` (QVector3D), `isArc`, `isClockwise`, `center` (QVector3D*),
`radius`, `plane` (XY/ZX/YZ), `speed`, `spindleSpeed`, `dwell`,
`isFastTraverse`, `isZMovement`, `isAbsolute`, `isMetric`, `lineNumber`.

### LineSegment  (`gcode/parser/linesegment.h`)
One rendered segment for the 3D view. Always a straight line (arcs are expanded).
Fields mirror PointSegment but add `start`/`end` (QVector3D), `drawn`, `vertexIndex`.

---

## Loading pipeline

```
File or QStringList
  ↓
GcodePreprocessorUtils::parseLine()    -- tokenise text → GCodeItem
  ↓
GCode list (QList<GCodeItem>)          -- stored as-is, raw text preserved
  ↓
GcodeParser::addCommand(item)          -- stateful parser → PointSegment
  ↓  (also sets item.commandNumber, item.isMovement)
GCodeViewParser::getLinesFromParser()  -- PointSegment list → LineSegment list
  ↓
GCodeLoaderData { gcode, viewParser }
```

`GCodeLoader` runs all three steps and emits `finished(result)`.
`GCodeLoader::update()` re-runs the view parse after an existing `GCode` is modified.

---

## GcodeParser  (`gcode/parser/gcodeparser.h`)

Stateful line-by-line interpreter. Tracks:
- Current position (absolute XYZ)
- Absolute / relative mode (G90 / G91)
- Absolute / relative IJK mode (G90.1 / G91.1)
- Active plane (G17/G18/G19 → XY/ZX/YZ)
- Metric / imperial (G21 / G20)
- Last speed (F), last spindle speed (S)
- Active canned cycle (G81-G83, G80 to cancel)

### State stack (push/pop)
Converters use `pushState()` / `popState()` to peek ahead without changing state:
```cpp
parser->pushState();
PointSegment *ps = parser->addCommand(item);  // peek
// read ps->point(), ps->center(), etc.
parser->popState();  // undo - ps is now deleted (dangling pointer, do not use)
```
Always save needed values from `ps` BEFORE calling `popState()`.

### Canned cycles
G81 (drill), G82 (drill+dwell), G83 (peck drill) are expanded inline into
addLinearPointSegment() calls. G80 cancels the active cycle.

---

## GCodeViewParser  (`gcode/parser/gcodeviewparser.h`)

Converts PointSegment list → LineSegment list for the 3D renderer.

- Arcs → many small LineSegments via `GcodePreprocessorUtils::generatePointsAlongArcBDring()`
- Lines → one LineSegment
- Tracks bounding box (min/max extremes)
- `getSimplifiedLines(precision)` merges short co-linear segments for LOD rendering

Arc precision is controlled by `arcPrecision` and `arcDegreeMode`:
- `arcDegreeMode = false`: max chord deviation in mm
- `arcDegreeMode = true`: degrees per segment

**Known minor issue**: `testLength()` (which updates `m_minLength`) is not called
for arc sub-segments, only for straight lines. `getResolution()` may be wrong
for files with only arcs.

---

## Converter architecture  (`gcode/converter/`)

There are two converter types. They differ in how they handle line count changes.

### `Converter` (base class, `converter.h`)
Use for **1:N** converters — one input line may produce multiple output lines.
The converter receives access to the full `GCode` list and may insert new items.

```cpp
class MyConverter : public Converter {
    bool convertLine(GCodeItem &item, GCode *gcode,
                     int currentIndex, GcodeParser *parser) override;
    bool needsParser() const override { return true; } // if you need position
};
```

- `item` is a **copy** of gcode[currentIndex]. Modify it freely.
- `gcode->insert(currentIndex + 1, seg)` to add extra lines after the current one.
  Subsequent iterations automatically process the inserted lines.
- Return `true` if item was modified (triggers write-back to gcode[currentIndex]).
- Return `false` if no change.

### `ConverterInterface` (interface, `converterinterface.h`)
Use for **heavy converters** that manage their own iteration (e.g. with complex
state that cannot be reset per-line). Must implement:
`setGCode()`, `convertNext()`, `convertAll()`, `reset()`, `hasMore()`.

Examples: `ApplyHeightmap`, `ShakingGCode`.

---

## Pipeline  (`converter/pipeline.h`)

Chains multiple `Converter` objects. Implements both the `Converter` interface
(for nesting) and provides standalone batch methods.

```cpp
Pipeline pipeline;
pipeline << new ArcsToLines(0.1);
pipeline << new FeedRateConverter(0.8);

// Mode A: convert everything at once, returns new GCode
pipeline.setGCode(source);
GCode *result = pipeline.convertAll();

// Mode B: pull / streaming
pipeline.setGCode(source);       // modifies source in-place
while (pipeline.hasMore()) {
    pipeline.convertNext(50);    // 50 lines at a time
}
```

### Parser mechanics in Pipeline / SingleConverter

For each line:
1. For each converter that `needsParser()`:
   - `parser->pushState()`        — save state before this line
   - `converter->convertLine(...)`— converter peeks / modifies
   - `parser->popState()`         — undo peek
2. After all converters: `parser->addCommand(gcode[i])` — advance past this line.

This guarantees that every converter that needs parser always sees the correct
machine position BEFORE the current line, regardless of whether earlier converters
modified it.

---

## SingleConverter  (`converter/singleconverter.h`)

Wraps one `Converter` object and provides the full `ConverterInterface`.
Use when you want to use a single `Converter` like `ArcsToLines` as a standalone
converter with the same API as `Pipeline`.

```cpp
SingleConverter sc(new ArcsToLines(0.1));
sc.setGCode(source);
GCode *result = sc.convertAll();
```

---

## ArcsToLines  (`converter/arcstolines.h`)

Converts G2/G3 arc movements to chains of G1 linear segments.

```cpp
// In pipeline (with other converters)
Pipeline p;
p << new ArcsToLines(0.1);          // 0.1 mm chord deviation
p << new FeedRateConverter(0.5);
p.setGCode(gcode);
GCode *result = p.convertAll();

// Standalone
SingleConverter sc(new ArcsToLines(1.0, true)); // 1 degree per segment
sc.setGCode(gcode);
GCode *result = sc.convertAll();
```

### How it works
1. `needsParser() = true` — requires parser to know start position.
2. On each G2/G3 line: saves start position from parser, parses arc parameters
   (endpoint, center, radius, plane), calls `generatePointsAlongArcBDring()`.
3. Replaces the arc item with G1 for the first sub-segment.
4. Inserts one GCodeItem for each remaining sub-segment via `gcode->insert()`.
5. All generated items have `command = "G1"`, `group = Movement`.

### Limitations
- Output is always **absolute (G90)** coordinates. Files using G91 relative mode
  for arcs may produce wrong results.
- Feed rate (F) is copied from the original arc to the first generated G1 only.
  Subsequent G1 segments in the same arc use the F value set by the first segment
  (which is modal in G-code and therefore correct).

---

## ApplyHeightmap  (`converter/applyheightmap.h`)

Applies Z-offset from a heightmap to all movement lines. Segments long moves so
the heightmap can be sampled at sufficient resolution.

Implements `ConverterInterface` directly. **Do not put in Pipeline** — use standalone.

```cpp
ApplyHeightmap converter(heightmap, 1.0); // 1 mm segments
converter.setGCode(original);
GCode *result = converter.convertAll();
```

### Notes
- Arcs (G2/G3) are converted to G1 segments (arc is already linearised).
- `segmentLine()` and `segmentArc()` apply the heightmap internally to each point.
  `processLine()` must NOT call `applyHeightmapToPoint()` again on the returned
  points (would double-apply the Z offset).
- `generateGCodeLine()` accepts `commandOverride` to force "G1" for arc segments.

---

## FusionRestoreRapidMovements  (`converter/fusionrestorerapidmovements.h`)

Ported from Tim Paterson's Fusion 360 post-processor add-in (`PostProcessAll.py`).
Detects G1 moves that Fusion 360 generated as feed moves but should be G0 (rapid),
and converts them. Adds `(Changed from: "...")` comments on modified lines.

```cpp
SingleConverter sc(new FusionRestoreRapidMovements());
sc.setGCode(source);
GCode *result = sc.convertAll();
```

### Algorithm

Five steps applied per line:

**A — Detect feed height.** The first two Z-only moves (no XY) establish `Zfeed`
(the clearance/feed height). If the second such move is G1, convert it to G0.

**B — Convert G1 to G0.** For any G1 with no XY component: if Z goes up, or Z is
at/above Zfeed, or feed rate is zero → convert to G0.
For G1 with XY but no Z: if current Z >= Zfeed → convert to G0.

**C — Restore G1 on modal lines.** After a G0 conversion, a modal line (no explicit
G-code) that moves below Zfeed is restored to G1 with explicit F.

**D — Add F parameter.** If a G1 line is missing F after a G0 conversion, append it.

**E — Adjust Zfeed upward.** If a cutting move is found above Zfeed, raise Zfeed to
avoid converting real cutting moves on the next pass.

### Bug fixes vs original Python

| Python bug | C++ fix |
|---|---|
| `Zcur >= Zlast` crashes when `Zlast is None` (first Z seen) → `except` disables all rapid processing | Guard with `!qIsNaN(m_zlast)` before comparison |
| `fNeedFeed` not cleared when line already has F (step D) | Condition is `m_lastMotionGcode != 0 && m_needFeed` — handled correctly |

### Limitations

- **G90 (absolute mode) only.** G91 (incremental) is not supported.
- **Experimental.** May misidentify moves near the feed height boundary.
- **Two clearance moves required for safe operation.** With only one G0 Z-only move
  before a plunge, Step A converts the plunge to G0 (tool crash risk). The algorithm
  requires at least two clearance-level Z-only moves before the first plunge.

---

## ExampleConverters  (`converter/exampleconverter.h`)

- `FeedRateConverter` — multiply all F values by a factor.
- `CoordinateOffsetConverter` — add XYZ offset to all absolute movement commands.
- `SafeSpindleStopConverter` — append warning comment after M5 if next is G0.
- `MovementOptimizerConverter` — demonstration of lookahead and full-gcode access.

---

## ShakingGCode  (`converter/shakinggcode.h`)

Test/demo converter. Segments lines and adds random XYZ offsets. Linear moves only,
arcs pass through unchanged.

**Known issue**: `getRandomFeedRate()` is defined but never called — the feed rate
variation feature (±20%) is not actually applied.

---

## Bug fixes applied (2025-03)

| # | Bug | Files |
|---|---|---|
| 1 | `ArcsToLines` used old interface, did not compile | `arcstolines.h/cpp` — full rewrite |
| 2 | Parser not advanced for unmodified lines in Pipeline/SingleConverter | `pipeline.cpp`, `singleconverter.cpp` |
| 3 | Double `reparseLine()` call in `Pipeline::convertNext()` | `pipeline.cpp` |
| 4 | Double heightmap applied to short lines in `ApplyHeightmap` | `applyheightmap.cpp` |
| 5 | Arc segments output as G2/G3 without arc params (invalid G-code) | `applyheightmap.h/cpp` |
| 6 | QList reference invalidation when a converter inserts lines | `pipeline.cpp`, `singleconverter.cpp` |

**Fix 2+3 (core pattern)**: `processLine()` now always advances the parser after
all converters finish. `reparseLine()` is removed. `convertAll()`/`convertNext()`
no longer call `reparseLine()` themselves.

**Fix 6 (reference safety)**: `processLine()` and `convertAll()` take a local copy
`GCodeItem item = (*gcode)[i]` before running converters. After converters finish,
the copy is written back if modified. This prevents dangling references when
`gcode->insert()` causes QList to reallocate.

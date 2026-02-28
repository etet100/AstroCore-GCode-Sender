// Example usage of ShakingGCode converter
// Demonstrates how to shake/distort G-code paths for testing

#include "core/gcode/converter/shakinggcode.h"
#include "core/gcode/gcode.h"
#include <QDebug>

void basicExample()
{
    qDebug() << "=== Basic ShakingGCode Example ===";

    // Assume we have loaded G-code
    GCode* originalGCode = new GCode();
    // ... load from file or create programmatically

    // Create shaker with default settings
    ShakingGCode shaker;
    shaker.setGCode(originalGCode);

    // Convert
    GCode* result = shaker.convertAll();

    qDebug() << "Original lines:" << originalGCode->count();
    qDebug() << "Shaken lines:" << result->count();
}

void customSettingsExample()
{
    qDebug() << "=== Custom Settings Example ===";

    GCode* originalGCode = new GCode();
    // ... load G-code

    // Configure shaker
    ShakingGCode shaker(
        3.0,   // Segment every 3mm
        0.5    // ±0.5mm random offset
    );

    shaker.setFeedRateVariation(0.1);  // ±10% feed rate
    shaker.setSeed(42);                // Reproducible results

    shaker.setGCode(originalGCode);
    GCode* result = shaker.convertAll();

    qDebug() << "Settings: segment=3mm, offset=±0.5mm, feedVar=±10%";
    qDebug() << "Result:" << result->count() << "lines";
}

void progressTrackingExample()
{
    qDebug() << "=== Progress Tracking Example ===";

    GCode* originalGCode = new GCode();
    // ... load G-code

    ShakingGCode shaker;

    // Connect progress signal
    QObject::connect(&shaker, &ShakingGCode::progressChanged,
        [](int current, int total) {
            int percent = (current * 100) / total;
            qDebug() << "Progress:" << percent << "%"
                     << "(" << current << "/" << total << ")";
        });

    shaker.setGCode(originalGCode);
    GCode* result = shaker.convertAll();

    qDebug() << "Conversion complete!";
}

void pullModeExample()
{
    qDebug() << "=== Pull Mode Example ===";

    GCode* gcode = new GCode();
    // ... load G-code

    ShakingGCode shaker;
    shaker.setGCode(gcode);

    // Process in batches
    int batchSize = 100;
    while (shaker.hasMore()) {
        int processed = shaker.convertNext(batchSize);
        qDebug() << "Processed" << processed << "lines, position:"
                 << shaker.currentPosition() << "/" << shaker.totalLines();
    }

    qDebug() << "All lines processed!";
}

void compareOriginalAndShaken()
{
    qDebug() << "=== Comparison Example ===";

    // Create simple test G-code
    GCode* original = new GCode();
    GCodeItem item;
    item.line = "G1 X100.0 Y100.0 Z-5.0 F1000";
    item.command = "G1";
    item.isMovement = true;
    *original << item;

    qDebug() << "Original:" << item.line;

    // Shake it
    ShakingGCode shaker(10.0, 0.8);  // Segment every 10mm, ±0.8mm offset
    shaker.setFeedRateVariation(0.15);  // ±15%
    shaker.setSeed(12345);

    shaker.setGCode(original);
    GCode* shaken = shaker.convertAll();

    qDebug() << "Shaken output (" << shaken->count() << "lines):";
    for (int i = 0; i < shaken->count(); i++) {
        qDebug() << "  " << shaken->at(i).line;
    }
}

void extremeShakeExample()
{
    qDebug() << "=== Extreme Shake Example ===";

    GCode* original = new GCode();
    // ... load G-code

    // Very aggressive settings
    ShakingGCode shaker(
        1.0,   // Very fine segments (every 1mm)
        2.0    // Large random offset (±2mm)
    );
    shaker.setFeedRateVariation(0.5);  // Wild feed variation (±50%)

    shaker.setGCode(original);
    GCode* result = shaker.convertAll();

    qDebug() << "Extreme shake created" << result->count() << "lines";
    qDebug() << "WARNING: This would look crazy in visualization!";
}

void subtleVibrationsExample()
{
    qDebug() << "=== Subtle Vibrations Example ===";

    GCode* original = new GCode();
    // ... load G-code

    // Simulate realistic machine vibrations
    ShakingGCode shaker(
        10.0,  // Coarse segments
        0.05   // Very small offset (±0.05mm = ±50 microns)
    );
    shaker.setFeedRateVariation(0.02);  // Minimal feed variation (±2%)

    shaker.setGCode(original);
    GCode* result = shaker.convertAll();

    qDebug() << "Subtle vibration simulation created" << result->count() << "lines";
    qDebug() << "Offsets: ±50 microns (realistic machine tolerance)";
}

// Main function to run all examples
int main(int argc, char *argv[])
{
    Q_UNUSED(argc);
    Q_UNUSED(argv);

    qDebug() << "ShakingGCode Converter - Usage Examples\n";

    basicExample();
    qDebug() << "";

    customSettingsExample();
    qDebug() << "";

    progressTrackingExample();
    qDebug() << "";

    pullModeExample();
    qDebug() << "";

    compareOriginalAndShaken();
    qDebug() << "";

    extremeShakeExample();
    qDebug() << "";

    subtleVibrationsExample();

    return 0;
}

/*
EXPECTED OUTPUT:

=== Basic ShakingGCode Example ===
Original lines: 1000
Shaken lines: 15234

=== Custom Settings Example ===
Settings: segment=3mm, offset=±0.5mm, feedVar=±10%
Result: 18567 lines

=== Progress Tracking Example ===
Progress: 5% (50/1000)
Progress: 10% (100/1000)
...
Progress: 100% (1000/1000)
Conversion complete!

=== Pull Mode Example ===
Processed 100 lines, position: 100/1000
Processed 100 lines, position: 200/1000
...
All lines processed!

=== Comparison Example ===
Original: G1 X100.0 Y100.0 Z-5.0 F1000
Shaken output (15 lines):
  G1 X10.234 Y9.876 Z-4.923 F1042.3
  G1 X19.876 Y20.123 Z-5.087 F987.6
  ...
  G1 X99.654 Y100.234 Z-4.956 F1023.1

=== Extreme Shake Example ===
Extreme shake created 156782 lines
WARNING: This would look crazy in visualization!

=== Subtle Vibrations Example ===
Subtle vibration simulation created 12456 lines
Offsets: ±50 microns (realistic machine tolerance)
*/

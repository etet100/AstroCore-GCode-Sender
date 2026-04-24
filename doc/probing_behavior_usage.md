# ProbingBehavior - Usage Documentation

## Overview

ProbingBehavior implements a professional two-phase probing sequence for CNC machines:
1. **Fast Probe** - Quick initial contact detection
2. **Retract** - Pull back from surface
3. **Slow Probe** - Precise final measurement

## Features

- ✅ Two-phase probing (fast + slow) for precision
- ✅ Automatic retract between probes
- ✅ Contact detection verification
- ✅ Optional Z-axis zeroing at probe position
- ✅ Safe position return
- ✅ Alarm handling
- ✅ Full logging and error reporting

## Usage Example

### Basic Usage (Default Parameters)

```cpp
// Simple probe with default settings
emit transition(this, new ProbingBehavior());
```

### Custom Parameters

```cpp
ProbingBehavior::ProbeParameters params;
params.fastFeedRate = 300.0;      // Fast probe at 300 mm/min
params.slowFeedRate = 30.0;       // Slow probe at 30 mm/min
params.maxDistance = 50.0;        // Maximum probe distance 50mm
params.retractDistance = 3.0;     // Retract 3mm between probes
params.safeDistance = 10.0;       // Move 10mm up after probing
params.setZeroAtProbe = true;     // Set Z=0 at probed position
params.useAbsolute = true;        // Return to G90 after probing

emit transition(this, new ProbingBehavior(params));
```

### Using ProbeAction

```cpp
// From IdleBehavior or other state
ProbeAction::ProbeParameters params;
params.fastFeedRate = 200.0;
params.slowFeedRate = 50.0;
params.maxDistance = 30.0;

ProbeAction probeAction(params);
// This will trigger transition to ProbingBehavior with custom params
```

## Probing Sequence

1. **Initial Setup**
   - Switch to relative positioning (G91)
   - Ensure metric units (G21)

2. **Fast Probe**
   - Command: `G38.2 Z-{maxDistance} F{fastFeedRate}`
   - Purpose: Quick surface detection
   - Example: `G38.2 Z-30 F200`

3. **Retract**
   - Command: `G0 Z{retractDistance}`
   - Purpose: Clear the surface
   - Example: `G0 Z2`

4. **Slow Probe**
   - Command: `G38.2 Z-{retractDistance+1} F{slowFeedRate}`
   - Purpose: Precise measurement
   - Example: `G38.2 Z-3 F50`

5. **Set Zero** (optional)
   - Command: `G92 Z0`
   - Purpose: Set current position as Z=0

6. **Move to Safe**
   - Command: `G0 Z{safeDistance}`
   - Purpose: Lift probe to safe height
   - Example: `G0 Z5`

7. **Restore State**
   - Return to absolute positioning (G90) if requested

## Response Format

Probe responses from GRBL/uCNC:
```
[PRB:0.000,0.000,-8.530:1]
```

Format: `[PRB:x,y,z:success]`
- `x,y,z` - Coordinates where probe contacted
- `success` - 1 = contact detected, 0 = no contact

## Error Handling

### Probe Failure (No Contact)
- Alarm codes: 4 or 5 (GRBL_ALARM_PROBE_FAIL_1/2)
- Behavior: Safe retract, transition to AlarmBehavior
- Signal: `probeFailed("No contact detected")`

### Alarm During Probing
- Any alarm stops the sequence
- Automatic transition to AlarmBehavior
- Signal: `probeFailed(QString reason)`

### Command Errors
- Parse errors, command rejections
- Safe retract attempted
- Transition back to previous state

## Signals

```cpp
// Emitted on successful probe completion
void probeCompleted(QVector3D position);

// Emitted on probe failure
void probeFailed(QString reason);
```

## Logging

All operations are logged with context:
```
[Probing] Starting probing sequence...
[Probing] Fast probe: G38.2 Z-30.000 F200.0
[Probing] Fast probe contact at Z=-8.530
[Probing] Retracting: 2.000mm
[Probing] Slow probe: G38.2 Z-3.000 F50.0
[Probing] Precise probe contact at Z=-8.532
[Probing] Z axis zeroed at probe position
[Probing] Moving to safe position: +5.000mm
[Probing] Probing completed successfully
```

## State Machine Stages

```cpp
enum class ProbeStage {
    InitialSetup,           // G91 G21
    FastProbe,              // Send fast probe command
    FastProbeWait,          // Wait for fast probe response
    Retract,                // Send retract command
    RetractWait,            // Wait for retract complete
    SlowProbe,              // Send slow probe command
    SlowProbeWait,          // Wait for slow probe response
    SetZero,                // G92 Z0 (optional)
    MoveToSafe,             // Move up to safe height
    Completed               // Done, return to previous state
};
```

## Best Practices

1. **Always use tool length sensor** - Ensure probe plate is connected
2. **Set appropriate feed rates** - Fast: 150-300 mm/min, Slow: 30-50 mm/min
3. **Check max distance** - Don't exceed your machine's Z travel
4. **Use setZeroAtProbe carefully** - Make sure this is what you want
5. **Monitor logs** - Check for contact detection

## Integration Example

```cpp
// In your UI code
void MainWindow::onProbeButtonClicked() {
    if (currentState->name() != "IdleBehavior") {
        log("Can only probe from Idle state");
        return;
    }

    ProbingBehavior::ProbeParameters params;
    params.fastFeedRate = ui->spinFastFeed->value();
    params.slowFeedRate = ui->spinSlowFeed->value();
    params.maxDistance = ui->spinMaxDistance->value();
    params.setZeroAtProbe = ui->checkSetZero->isChecked();

    ProbeAction probeAction(params);
    communicator->performAction(probeAction);
}

// Connect signals
connect(probingBehavior, &ProbingBehavior::probeCompleted,
        this, [this](QVector3D pos) {
    ui->labelProbedZ->setText(QString::number(pos.z(), 'f', 3));
    log("Probing successful");
});

connect(probingBehavior, &ProbingBehavior::probeFailed,
        this, [this](QString reason) {
    QMessageBox::warning(this, "Probe Failed", reason);
});
```

## Troubleshooting

### Probe doesn't trigger
- Check electrical connections
- Verify probe plate is properly grounded
- Test with multimeter

### "No contact detected" error
- Increase `maxDistance`
- Check if probe is too far from surface
- Verify probe is working (test continuity)

### Machine moves but doesn't probe
- Controller might not support G38.2
- Check GRBL/uCNC version and configuration
- Review controller documentation

### Inconsistent measurements
- Decrease `slowFeedRate` for more precision
- Increase `retractDistance` to fully clear surface
- Check for mechanical play in Z-axis

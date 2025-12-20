# Command Autocomplete

## Feature

Console (`partMainConsole`) has autocomplete for predefined commands.

## How to use

### Basic usage

1. Start typing a command with `:` prefix, for example `:s`
2. Press **Tab** - first matching command appears (for example `:start`)
3. Press **Tab** again - move to next command (for example `:status`)
4. Press **Tab** again - cycle back to first command
5. Press **Escape** - cancel autocomplete and return to original text (`:s`)

### Default commands

- `start`
- `stop`
- `status`
- `pause`
- `open`
- `reset`

### Setting custom commands

```cpp
partMainConsole* console = new partMainConsole(parent);
console->setPredefinedCommands({"start", "stop", "restart", "reload"});
```

## Example workflow

```
User types: `:s`
Press Tab  → `:start`
Press Tab  → `:status`
Press Tab  → `:stop`
Press Tab  → `:start` (wraps around)
Press Esc  → `:s` (cancelled, back to original)

User types: `:sta`
Press Tab  → `:start`
Press Tab  → `:status`
Press Tab  → `:start` (only 2 matches)
```

## Implementation details

- Autocomplete is implemented in `partMainConsole` class using event filter on ComboBox
- Works only for commands starting with `:`
- Search is case-insensitive
- Results are sorted alphabetically
- Any key press (except Tab and Escape) cancels autocomplete
- After cancel, you can continue typing and use Tab again
- Does not affect other `ComboBox` usage in the application

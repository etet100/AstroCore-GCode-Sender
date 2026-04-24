# Configuration module

This note describes how configuration is split between core and non-core
layers in G-Pilot.

## Layout

```
src/gpilot/
├── core/
│   ├── config/
│   │   ├── configuration.{h,cpp}     Registry + CORE module owner
│   │   ├── registry.h                Custom type (de)serialisation
│   │   ├── module/                   CORE modules only
│   │   │   ├── abstractconfigurationmodule.{h,cpp}
│   │   │   ├── configurationconnection.{h,cpp}
│   │   │   ├── configurationjogging.{h,cpp}
│   │   │   ├── configurationmachine.{h,cpp}
│   │   │   ├── configurationparser.{h,cpp}
│   │   │   └── configurationsender.{h,cpp}
│   │   └── persistence/              INI / JSON / XML backends
│   └── heightmap/
│       └── configurationheightmap.{h,cpp}
├── modules/
│   ├── ai/
│   │   └── configurationai.{h,cpp}
│   └── pendant/
│       └── configurationpendant.{h,cpp}
└── ui/
    └── config/
        ├── uiconfigs.{h,cpp}         Holder for the four UI modules below
        ├── configurationui.{h,cpp}
        ├── configurationconsole.{h,cpp}
        ├── configurationmacros.{h,cpp}
        └── configurationvisualizer.{h,cpp}
```

## Registration model

`Configuration` owns the CORE modules as member fields and also keeps a list
of every registered module. Non-core modules live in their own directories
and plug in via `registerModule(AbstractConfigurationModule*)`.

- **Eager registration** (90% of cases): the module registers itself before
  `Configuration::init()`. All registered modules are loaded in one pass when
  `init()` runs.
- **Lazy registration**: if `init()` has already run, `registerModule()` opens
  the persistence backend and loads just that module immediately — useful
  when a feature is created on demand and wants its config right away.

Example (`main.cpp`):

```cpp
Configuration& cfg = Core::instance().configuration();

// Eager — register before init():
UiConfigs::instance().registerAll(cfg);
ConfigurationAI::registerWith(cfg);
ConfigurationPendant::registerWith(cfg);
ConfigurationHeightmap::registerWith(cfg);

cfg.init(appPath, configType);   // loads all registered modules at once
```

## Accessors

Module → access pattern

| Module | Access |
|---|---|
| Core (connection, machine, sender, parser, jogging) | `Core::instance().configuration().xxxModule()` |
| UI (ui, console, visualizer, macros) | `UiConfigs::instance().xxx()` |
| AI, Pendant, Heightmap | `ConfigurationXxx::instance()` |

## File format

The config file (`config.json` / `config.ini` / `config.xml`) stores each
module in a section named after `getSectionName()`. Moving source files
between directories does not change the file format — section names stay the
same, and persisters already use read-modify-write, so sections of modules
that are not currently registered are preserved on save.

## Adding a new non-core config module

1. Create `ConfigurationXxx` deriving from `AbstractConfigurationModule`
   with `Q_PROPERTY` declarations and a unique `getSectionName()`.
2. Add a `static ConfigurationXxx& instance()` and
   `static void registerWith(Configuration&)` following the pattern in
   `modules/ai/configurationai.h`.
3. Call `ConfigurationXxx::registerWith(cfg)` in `main.cpp` before
   `cfg.init(...)`, or from wherever the feature is activated for lazy
   loading.

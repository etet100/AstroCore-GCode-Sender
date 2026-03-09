---
title: Testing and Examples
---

# Testing and Examples

The repository contains a few small test and example resources that can be useful when experimenting with G-Pilot.

## G-code examples

In the `testing/gcode` folder you can find sample G-code files, for example:

- `1.nc`, `1warped.nc`
- `3d.nc`
- `arc.nc`, `arc2.nc`
- `simple.nc`

They are useful for testing visualisation, heightmaps and streaming logic.

## Shaking G-code example

In `testing/examples/shakinggcode_example.cpp` there is a C++ example that shows how shaking G-code can be generated or processed.

The repository also contains a test:

- `tests/test_shakinggcode.cpp`

This can be used as a reference when working on the shaking G-code feature.

## OpenGL / WebGL experiments

The `testing/gl` folder contains experimental OpenGL/WebGL projects (TypeScript/JavaScript, HTML). They are not part of the main application but can be useful as reference for 3D visualisation experiments.

## How to use the tests

- Use the G-code files as input for G-Pilot to test visualisation and streaming
- Use the C++ test sources as reference when modifying related modules
- Extend the tests or add new ones if you change behaviour in the corresponding features

The testing infrastructure is intentionally simple and focused on concrete examples directly related to CNC workflows.

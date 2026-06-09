# Test CLI Commands

Run all commands from the project root:

```text
travis/
```

Do not run test commands inside `tests/`.

## Normal commands you will usually use

### 1. Configure once

```powershell
cmake --preset mingw-debug
```

Use this:
- first time only
- or after changing `CMakeLists.txt`
- or after changing `CMakePresets.json`

### 2. Build

```powershell
cmake --build --preset mingw-debug
```

Use this after code changes.

### 3. Run all tests

```powershell
ctest --preset mingw-debug
```

## Standard daily flow

Usually you run these two:

1. build
2. test

```powershell
cmake --build --preset mingw-debug
ctest --preset mingw-debug
```

## Run grouped tests

### Business logic / service tests

These validate repository-backed business rules and service behavior.

```powershell
ctest --test-dir build/mingw-debug -R "project_service_test|session_service_test|structure_service_test|execution_service_test|result_service_test|video_service_test|inspection_clip_service_test" --output-on-failure
```

### Core / media smoke tests

These validate native app startup, GStreamer runtime health, source discovery, playback workflow, and recording recovery behavior.

```powershell
ctest --test-dir build/mingw-debug -R "media_runtime_test|source_discovery_test|app_context_test|playback_workflow_test|recording_recovery_service_test" --output-on-failure
```

## Run one test only

Example:

```powershell
ctest --test-dir build/mingw-debug -R project_service_test --output-on-failure
```

Replace `project_service_test` with:

- `session_service_test`
- `structure_service_test`
- `execution_service_test`
- `result_service_test`
- `video_service_test`
- `inspection_clip_service_test`
- `media_runtime_test`
- `source_discovery_test`
- `app_context_test`
- `playback_workflow_test`
- `recording_recovery_service_test`

## If build files do not exist yet

```powershell
cmake --preset mingw-debug
cmake --build --preset mingw-debug
ctest --preset mingw-debug
```

## Simple rule

- run everything from project root
- do not go into `tests/`
- normal command for daily use:

```powershell
cmake --build --preset mingw-debug
ctest --preset mingw-debug
```

## Current preset

This project currently uses:

- preset name: `mingw-debug`
- Qt path: `C:/Qt/6.8.3/mingw_64`
- generator: `Ninja`

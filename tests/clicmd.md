# Test CLI Commands

This project uses:

- `Qt Test` for writing tests
- `CTest` for running registered test executables
- `CMake + Ninja + MinGW` for the current local build setup

Run all commands from:

```text
travis/
```

Do not run these commands from `tests/`.

## 1. Configure the build

```powershell
cmake --preset mingw-debug
```

What it does:

- creates the build folder: `build/mingw-debug`
- configures the project with the Qt MinGW kit
- enables the test targets under `tests/`

You usually run this once first, or again after changing CMake files.

## 2. Build the project and all tests

```powershell
cmake --build --preset mingw-debug
```

What it does:

- builds app libraries
- builds all test executables too

Use this after code changes.

## 3. Run all tests

```powershell
ctest --preset mingw-debug
```

What it does:

- runs every registered Qt test
- prints failure details directly in terminal

Yes, normally you run both:

1. build
2. test

Example normal flow:

```powershell
cmake --build --preset mingw-debug
ctest --preset mingw-debug
```

## 4. Run one test only

Example:

```powershell
ctest --test-dir build/mingw-debug -R project_service_test --output-on-failure
```

You can replace `project_service_test` with:

- `session_service_test`
- `structure_service_test`
- `execution_service_test`
- `result_service_test`
- `video_service_test`
- `inspection_clip_service_test`
- `app_context_test`

## 5. Rebuild after code changes

```powershell
cmake --build --preset mingw-debug
ctest --preset mingw-debug
```

## Important

This preset currently uses:

- Qt path: `C:/Qt/6.8.3/mingw_64`
- generator: `Ninja`

Do not use the Visual Studio generator with this Qt install, because this Qt package is the MinGW build.

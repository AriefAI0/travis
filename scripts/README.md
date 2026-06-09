# Travis Qt Dev Scripts

This folder contains small CLI helpers for running the Qt app without Qt Creator.

## Fast Dev Run

Build and launch the app from the CMake/Ninja build folder:

PowerShell:

```powershell
.\scripts\run-dev.ps1
```

Git Bash:

```bash
./scripts/run-dev.sh
```

## Validate Runtime Only

Check the build folder, Qt bin path, and bundled GStreamer runtime without opening the app:

PowerShell:

```powershell
.\scripts\run-dev.ps1 -NoLaunch
```

Git Bash:

```bash
./scripts/run-dev.sh -NoLaunch
```

## Launch Without Rebuild

Use this after a successful build when you only want to reopen the app:

PowerShell:

```powershell
.\scripts\run-dev.ps1 -NoBuild
```

Git Bash:

```bash
./scripts/run-dev.sh -NoBuild
```

## Custom Paths

Override paths if the local machine uses a different build or Qt install:

PowerShell:

```powershell
.\scripts\run-dev.ps1 -BuildDir build/mingw-debug -QtRoot C:/Qt/6.8.3/mingw_64 -GStreamerRuntimeRoot runtime/gstreamer/1.0/msvc_x86_64
```

Git Bash:

```bash
./scripts/run-dev.sh -BuildDir build/mingw-debug -QtRoot C:/Qt/6.8.3/mingw_64 -GStreamerRuntimeRoot runtime/gstreamer/1.0/msvc_x86_64
```

## Notes

- Runtime environment changes are process-local.
- GStreamer DLL and plugin paths are set by the script.
- Use `./scripts/run-dev.sh` from Git Bash because backslashes in `.\scripts\run-dev.ps1` are treated as escape characters.
- Deploy/release workflow details live in `../../planning/migration-refactorize-plan/qt-dev-run-deploy-plan.md`.

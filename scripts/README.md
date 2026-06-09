# Travis Qt Dev Scripts

This folder contains small CLI helpers for running the Qt app without Qt Creator.

## Fast Dev Run

Build and launch the app from the CMake/Ninja build folder:

Developer PowerShell or PowerShell:

```powershell
.\scripts\run-dev.ps1
```

cmd.exe:

```cmd
"C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat" -arch=x64 && powershell.exe -NoProfile -ExecutionPolicy Bypass -File scripts\run-dev.ps1
```

Git Bash:

```bash
./scripts/run-dev.sh
```

If `.\scripts\run-dev.ps1` opens the file as text, you are not in PowerShell. Use the `cmd.exe` command above.

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
.\scripts\run-dev.ps1 -BuildDir build/msvc-debug -QtRoot C:/Qt/6.8.3/msvc2022_64 -GStreamerRuntimeRoot runtime/gstreamer/1.0/msvc_x86_64
```

Git Bash:

```bash
./scripts/run-dev.sh -BuildDir build/msvc-debug -QtRoot C:/Qt/6.8.3/msvc2022_64 -GStreamerRuntimeRoot runtime/gstreamer/1.0/msvc_x86_64
```

## Install Qt MSVC With aqtinstall

Best practical setup:

```powershell
python -m pip install --user aqtinstall
```

Then use it without PATH issues:

```powershell
python -m aqt install-qt windows desktop 6.8.3 win64_msvc2022_64 -O C:/Qt
```

## Configure MSVC Build

Run this from **Developer PowerShell for VS 2022** after installing Qt `win64_msvc2022_64`:
`.\scripts\run-dev.ps1` runs directly only from PowerShell or Developer PowerShell.

```powershell
cmake --preset msvc-debug
cmake --build --preset msvc-debug
.\scripts\run-dev.ps1
```

## Legacy MinGW Run

Use this only when intentionally testing the old MinGW build:

```powershell
.\scripts\run-dev.ps1 -BuildDir build/mingw-debug -QtRoot C:/Qt/6.8.3/mingw_64
```

## Notes

- Runtime environment changes are process-local.
- GStreamer DLL and plugin paths are set by the script.
- Default development target is `build/msvc-debug` so Qt, MSVC, and bundled GStreamer use the same Windows compiler family.
- Use `./scripts/run-dev.sh` from Git Bash because backslashes in `.\scripts\run-dev.ps1` are treated as escape characters.
- Deploy/release workflow details live in `../../planning/migration-refactorize-plan/qt-dev-run-deploy-plan.md`.

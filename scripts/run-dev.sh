#!/usr/bin/env bash
# Runs the Qt dev PowerShell script from Git Bash/MSYS terminals.

set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

pwsh_bin=""
if command -v pwsh >/dev/null 2>&1; then
    pwsh_bin="pwsh"
elif command -v powershell.exe >/dev/null 2>&1; then
    pwsh_bin="powershell.exe"
else
    echo "PowerShell was not found. Run this from PowerShell or install PowerShell." >&2
    exit 1
fi

"$pwsh_bin" -NoProfile -ExecutionPolicy Bypass -File "$script_dir/run-dev.ps1" "$@"

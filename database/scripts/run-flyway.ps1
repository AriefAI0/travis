param(
    [Parameter(Mandatory = $false)]
    [ValidateSet("info", "migrate", "validate", "repair")]
    [string]$Command = "migrate",

    [Parameter(Mandatory = $false)]
    [string]$FlywayPath
)

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
$configPath = Join-Path $repoRoot "database\flyway.conf"

if (-not (Test-Path $configPath)) {
    Write-Error "Flyway config not found: $configPath"
    exit 1
}

if ($FlywayPath) {
    $flywayExe = $FlywayPath
} else {
    $flywayCmd = Get-Command flyway -ErrorAction SilentlyContinue
    if ($flywayCmd) {
        $flywayExe = $flywayCmd.Source
    } else {
        Write-Error "Flyway executable not found on PATH. Install Flyway or pass -FlywayPath."
        exit 1
    }
}

& $flywayExe "-configFiles=$configPath" $Command
$exitCode = $LASTEXITCODE

if ($exitCode -ne 0) {
    exit $exitCode
}

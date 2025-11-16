# pg_ask Extension Installer for Windows PowerShell
# Usage: .\install.ps1 [-Version "latest"] [-PgVersion "16"]

param(
    [string]$Version = "latest",
    [string]$PgVersion = "16"
)

$ErrorActionPreference = "Stop"

# Configuration
$RepoUrl = "https://github.com/$($env:GITHUB_REPOSITORY)"
$ReleaseUrl = "$RepoUrl/releases/download"
$Platform = "windows-x64"
$ExtensionFile = "pg_ask.dll"

# Helper functions
function Write-Info { Write-Host "[INFO] $args" -ForegroundColor Green }
function Write-Warn { Write-Host "[WARN] $args" -ForegroundColor Yellow }
function Write-Error { Write-Host "[ERROR] $args" -ForegroundColor Red; exit 1 }

Write-Info "Installing pg_ask extension (version: $Version, PostgreSQL: $PgVersion)"

# Find PostgreSQL installation
$PgPaths = @(
    "C:\Program Files\PostgreSQL\$PgVersion",
    "C:\Program Files (x86)\PostgreSQL\$PgVersion"
)

$PgInstall = $null
foreach ($path in $PgPaths) {
    if (Test-Path "$path\bin\pg_config.exe") {
        $PgInstall = $path
        break
    }
}

if ($null -eq $PgInstall) {
    Write-Error "PostgreSQL $PgVersion not found. Please install PostgreSQL or specify the correct version."
}

Write-Info "Using PostgreSQL at: $PgInstall"

$PkgLibDir = "$PgInstall\lib"
$ShareDir = "$PgInstall\share\extension"

# Create temporary directory
$TempDir = New-TemporaryFile | ForEach-Object { Remove-Item $_; New-Item -ItemType Directory -Path $_ }

try {
    Write-Info "Downloading extension files..."
    
    $DownloadUrl = "$ReleaseUrl/$Version/$Platform/$ExtensionFile"
    Invoke-WebRequest -Uri $DownloadUrl -OutFile "$TempDir\$ExtensionFile" -ErrorAction Stop
    Invoke-WebRequest -Uri "$ReleaseUrl/$Version/$Platform/pg_ask.control" -OutFile "$TempDir\pg_ask.control" -ErrorAction Stop
    Invoke-WebRequest -Uri "$ReleaseUrl/$Version/$Platform/pg_ask--1.0.sql" -OutFile "$TempDir\pg_ask--1.0.sql" -ErrorAction Stop
    
    Write-Info "Installing extension files..."
    
    # Check if running as admin
    $IsAdmin = ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
    if (-not $IsAdmin) {
        Write-Warn "This script requires administrator privileges to install extension files."
        Write-Host "Please run PowerShell as Administrator and try again."
        exit 1
    }
    
    Copy-Item "$TempDir\$ExtensionFile" "$PkgLibDir\pg_ask.dll" -Force
    Copy-Item "$TempDir\pg_ask.control" "$ShareDir\pg_ask.control" -Force
    Copy-Item "$TempDir\pg_ask--1.0.sql" "$ShareDir\pg_ask--1.0.sql" -Force
    
    Write-Info "Extension installed successfully!"
    Write-Info "Create the extension with: CREATE EXTENSION pg_ask;"
}
finally {
    Remove-Item -Recurse -Force $TempDir -ErrorAction SilentlyContinue
}

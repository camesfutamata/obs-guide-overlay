# Script de build do obs-guide-overlay para Windows
# Requer MSYS2 instalado em C:\msys64

$ErrorActionPreference = "Stop"
$MSYS2 = "C:\msys64\usr\bin\bash.exe"
$MINGW_GCC = "C:\msys64\mingw64\bin\gcc.exe"
$OBS_DIR = "C:\Program Files\obs-studio"
$PLUGIN_DIR = Split-Path -Parent $MyInvocation.MyCommand.Definition

function Run-MSYS2 {
    param([string]$cmd)
    & $MSYS2 -l -c $cmd
    if ($LASTEXITCODE -ne 0) { throw "Falhou: $cmd" }
}

Write-Host "=== Verificando MSYS2 ===" -ForegroundColor Cyan
if (-not (Test-Path $MSYS2)) {
    throw "MSYS2 nao encontrado em C:\msys64. Instale via: winget install MSYS2.MSYS2"
}

Write-Host "=== Atualizando MSYS2 e instalando GCC ===" -ForegroundColor Cyan
Run-MSYS2 "pacman --noconfirm -Sy 2>&1 | tail -5"
Run-MSYS2 "pacman --noconfirm -S mingw-w64-x86_64-gcc mingw-w64-x86_64-obs-studio 2>&1 | tail -20"

Write-Host "=== Preparando diretorios de build ===" -ForegroundColor Cyan
$BUILD_DIR = "$PLUGIN_DIR\build_output"
New-Item -ItemType Directory -Force -Path $BUILD_DIR | Out-Null

# Caminhos dentro do MSYS2 (convertidos para paths UNIX)
$SRC = "/c/Users/Denis/Desktop/obs_nrs/src/guide-overlay.c"
$OUT = "/c/Users/Denis/Desktop/obs_nrs/build_output/obs-guide-overlay.dll"
$INCLUDE = "/c/msys64/mingw64/include/obs"
$INCLUDE_GFX = "/c/msys64/mingw64/include"

Write-Host "=== Compilando plugin ===" -ForegroundColor Cyan
$compile_cmd = @"
gcc -O2 -shared -fPIC \
  -I/c/msys64/mingw64/include \
  -I/c/msys64/mingw64/include/obs \
  $SRC \
  -L/c/msys64/mingw64/lib \
  -lobs -lobs-frontend-api \
  -o $OUT \
  2>&1
"@
Run-MSYS2 $compile_cmd

Write-Host "=== Instalando plugin no OBS ===" -ForegroundColor Cyan
$PLUGIN_DEST = "$OBS_DIR\obs-plugins\64bit\obs-guide-overlay.dll"
$DATA_DEST   = "$OBS_DIR\data\obs-plugins\obs-guide-overlay"

Copy-Item "$BUILD_DIR\obs-guide-overlay.dll" $PLUGIN_DEST -Force
New-Item -ItemType Directory -Force -Path $DATA_DEST | Out-Null
Copy-Item "$PLUGIN_DIR\data\*" $DATA_DEST -Recurse -Force

Write-Host ""
Write-Host "=== Plugin instalado com sucesso! ===" -ForegroundColor Green
Write-Host "Reinicie o OBS e adicione a fonte 'Guide Overlay'" -ForegroundColor Green

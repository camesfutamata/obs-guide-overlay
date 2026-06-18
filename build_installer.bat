@echo off
chcp 65001 >nul
title Compilar Instalador — obs-guide-overlay

echo ============================================
echo  Compilando instalador obs-guide-overlay
echo ============================================
echo.

:: Verificar se o Inno Setup está instalado
if exist "C:\Program Files (x86)\Inno Setup 6\ISCC.exe" (
    echo [1/2] Compilando com Inno Setup...
    "C:\Program Files (x86)\Inno Setup 6\ISCC.exe" installer.iss
    if %errorlevel% equ 0 (
        echo   OK! Instalador gerado na pasta installer_output\
    ) else (
        echo   ERRO: Falha ao compilar com Inno Setup
    )
) else (
    echo [1/2] Inno Setup nao encontrado. Pule para NSIS...
)

echo.

:: Verificar se o NSIS está instalado
if exist "C:\Program Files (x86)\NSIS\makensis.exe" (
    echo [2/2] Compilando com NSIS...
    "C:\Program Files (x86)\NSIS\makensis.exe" installer.nsi
    if %errorlevel% equ 0 (
        echo   OK! Instalador gerado: obs-guide-overlay-setup-1.0.1.exe
    ) else (
        echo   ERRO: Falha ao compilar com NSIS
    )
) else (
    echo [2/2] NSIS nao encontrado.
)

echo.
echo ============================================
echo  Para compilar manualmente:
echo.
echo  Inno Setup: ISCC.exe installer.iss
echo  NSIS:       makensis.exe installer.nsi
echo ============================================
echo.
pause

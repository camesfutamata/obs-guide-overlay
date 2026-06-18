@echo off
setlocal
cd /d "%~dp0"
echo Compilando...
C:\msys64\usr\bin\bash.exe -l "C:\Users\Denis\Desktop\obs_nrs\compile2.sh"
if exist releases\v1.0.1\obs-guide-overlay.dll (
    echo Sucesso! releases\v1.0.1\obs-guide-overlay.dll criado
) else (
    echo ERRO: falha na compilacao
    pause & exit /b 1
)
pause

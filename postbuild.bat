@echo off
set mode=%1
set libfolder=%2
set destination=%3

set copyopts=/NJS /NP

:: Dlls
:: sfml-window-2
:: sfml-graphics-2
:: sfml-system-2

if "%mode%"=="Debug" (
    set sfmlDlls=sfml-*-d-2.dll
) else (
    set sfmlDlls=sfml-*-2.dll
    set copyopts=%copyopts% /XF *-d-*
)

call :cp %sfmlDlls% %libfolder% %destination% %copyopts%

:: resources
if not exist "%destination%resources" mkdir "%destination%resources"
copy resources\* "%destination%resources"
copy game.cfg "%destination%"

goto :eof


:cp
setlocal
robocopy %*
set rce=%errorlevel%
if %rce% LSS 8 (exit /B 0) else (exit %rce%)
endlocal & goto:eof
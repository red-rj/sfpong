set mode=%1
set libfolder=%2
set destination=%3

set copyopts=/NP /NS

:: sfml-window-2
:: sfml-graphics-2
:: sfml-system-2

if "%mode%"=="Debug" (
    set sfmlDlls=sfml-*-d-2.dll
) else (
    set sfmlDlls=sfml-*-2.dll
    set copyopts=%copyopts% /XF *-d-*
)

robocopy %sfmlDlls% %libfolder% %destination% %copyopts%

set rce=%errorlevel%
if %rce% EQU 1 (exit /B 0) else (exit /B %rce%)

@echo off

rem Get path to folder containing this file and remove the trailing backslash
set THISFILEPATH=%~dp0
set THISFILEPATH=%THISFILEPATH:~0,-1%

rem Set environment variable.
setx LEIASR_SDKROOT "%THISFILEPATH%"

pause
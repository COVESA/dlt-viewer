@echo off

set "missingArguments="
if "%~1" EQU "" set "missingArguments=1"
if "%~2" NEQ "PACKAGE_MAJOR_VERSION" (
    if "%~2" NEQ "PACKAGE_MINOR_VERSION" (
        if "%~2" NEQ "PACKAGE_PATCH_LEVEL" (
            set "missingArguments=1"
        )
    )
)

if defined missingArguments (
    call :printUsage
    goto :eof
)

set findFile="%1"
set findText="#define %2"

setlocal DisableDelayedExpansion
for /f "usebackq delims=" %%a in (`"findstr /C:%findText% %findFile%"`) do (
    set "foundLine=%%a"
    call :processLine foundLine findText
)
endlocal
goto :eof

:processLine
    setlocal EnableDelayedExpansion
    set "foundLine=!%1!"
    set findText=!%2:"=!
    set "foundValue=!foundLine:%findText%=!"
    call :trimString %foundValue% foundValue
    echo %foundValue%
    endlocal
goto :eof

:trimString
    set %2=%1
goto :eof

:printUsage
    echo Parse DLT Viewer version.h and return version number depending on requested part.
    echo Usage: parse_version.bat ^<path to version.h^> ^<PACKAGE_MAJOR_VERSION ^| PACKAGE_MINOR_VERSION ^| PACKAGE_PATCH_LEVEL^>
    exit /b 1

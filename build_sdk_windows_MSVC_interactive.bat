call build_sdk_windows_MSVC.bat
SET RETCODE=%ERRORLEVEL%
set /p name= Continue
exit /b %RETCODE%
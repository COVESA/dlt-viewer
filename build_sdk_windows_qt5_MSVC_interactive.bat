call build_sdk_windows_qt5_MSVC.bat
SET RETCODE=%ERRORLEVEL%
set /p name= Continue
exit /b %RETCODE%
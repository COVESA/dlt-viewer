call build_sdk_windows_MSVC_cmake.bat
SET RETCODE=%ERRORLEVEL%
set /p name= Continue
exit /b %RETCODE%
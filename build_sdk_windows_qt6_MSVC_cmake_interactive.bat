call build_sdk_windows_qt6_MSVC_cmake.bat
SET RETCODE=%ERRORLEVEL%
set /p name= Continue
exit /b %RETCODE%
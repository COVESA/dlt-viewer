# DLT Viewer: CMake + MS Visual Studio Code
- QMake and QT Creator are solid and good tools, tightly integrated with QT framework.
- CMake and MS VS Code are gaining more and more acceptance around dev community as strong **cross-platform** development environment.
    - With deep CMake and Ninja integration in modern Microsoft development tools, there is no need for MinGW/GCC to use CMake on Windows. Also no need to use CMake `Visual Studio generator`, Ninja can replace Make and MSBuild/NMake.
    - Future QMake development is stopped by QT project
        - https://www.qt.io/blog/qt-and-cmake-the-past-the-present-and-the-future
        - https://www.qt.io/blog/2019/08/07/technical-vision-qt-6
        - https://doc.qt.io/qt-5/cmake-get-started.html
- To encourage contribution to DLT Viewer, development environment is configured with CMake + VS Code. Old QMake/QT Creator based workflow is preserved.
- `cmake install` and `cpack` provide easy to maintain cross platform installation and packaging setup.
- CMake project configuration still allows build and development of `dlt-viewer`, as before with QMake.
- With Ninja parallel build, build time of the whole project is not a concern anymore.

# Windows development environment setup
For DLT Viewer development on **Linux** please see [Linux development environment setup](../linux/build.md)

Configuration:
- Windows 10/11 x64
- Administrator rights
- MS VS Code
    - https://code.visualstudio.com/download
    - Regular MS Visual Studio is not needed.
    - Make sure `code` command is runnable from cmd, otherwise add to PATH.
    - Disable [Telemetry](https://code.visualstudio.com/docs/getstarted/telemetry)
        - Use https://vscodium.com/ if you don't like Microsoft branding and Telemetry.
- Git for Windows
    - https://git-scm.com/download/win
    - Keep defaults.
    - On step 5 choose `Git from the command line and also from 3rd-party software`.
    - Make sure `git` command is runnable from cmd, otherwise add to PATH.
- CMake
    - https://cmake.org/download/
    - Download and install fresh CMake, do not use one bundled with VS Build Tools, with Visual Studio or VS Code extension. Always install latest.
    - Add `c:\Program Files\Cmake\bin` to PATH.
        - Control Panel / User Accounts / Change my environment variables / User variables / Double click "Path".
- Visual Studio **2017** Build Tools
    - https://aka.ms/vs/15/release/vs_buildtools.exe
    - **Do not use latest version of MS Build Tools** (2019 or 2022). Do not go to this page: [Microsoft C++ Build Tools](https://visualstudio.microsoft.com/visual-cpp-build-tools/). Using older VS 2017 Build Tools will produce package **compatible with broader range of platforms**.
    - During installation select Optional:
        - [x] Windows 10 SDK (10.0.17763.0)
        - [x] Visual C++ tools for CMake
        - [x] Testing tools Core features - Buildtools
        - [x] C++/CLI Support
    - No need to download and install MS Visual Studio (Community Edition)
- Ninja
    - Already bundled with VS Build Tools.
- QT 5.12.10
    - Later versions of QT5 will require using [QT Online installer](https://www.qt.io/download-qt-installer).
    - QT6 is not yet supported by DLT Viewer.
    - https://download.qt.io/official_releases/qt/5.12/5.12.10/qt-opensource-windows-x86-5.12.10.exe
    - Install requires QT account. Use private E-Mail to register for free.
    - Select
        - [x] *MSVC 2017 64-bit*
        - [x] *QT Charts*
        - [x] *QT Creator* - Needed only for visual editing of `.ui` files.
    - No need to build QT from sources.
    - Add `c:\Qt\Qt5.12.10\5.12.10\msvc2017_64\bin\` to PATH.
- VS Code extensions
    - Just copy extension name in extension search box
    - `ms-vscode.cpptools`
    - `ms-vscode.cmake-tools`
    - `eamodio.gitlens`
    - `tonka3000.qtvsctools`
        - This will allow QT debugging and visual editing of .ui files with QT Creator.
    - For remote editing of DLT Viewer source code **in Linux VM**
        - `ms-vscode-remote.remote-ssh`
        - This will allow remote editing, but not remote debugging of QT application inside Linux.
        - For debugging QT application inside Linux you need to [install VS Code inside Linux](../linux/build.md), and do not use this extension.
        - Configuration of SSH daemon on Linux and SSH client on Windows is needed
            - OpenSSH comes together with  Windows 10
                - C:\Windows\System32\OpenSSH\ssh.exe -V
                    - OpenSSH_for_Windows_8.1p1, LibreSSL 3.0.2
                - c:\Users\<user id>\.ssh\config
            - Another option is SSH client, which comes with "Git for Windows"
                - C:\Users\%%USER\AppData\Local\Programs\Git\usr\bin\ssh.exe -V
                    - OpenSSH_8.8p1, OpenSSL 1.1.1l  24 Aug 2021
- Setup `.vscode` folder
    - Copy content of `dlt-viewer\scripts\windows\.vscode_example\*.json` into `dlt-viewer\.vscode\`
    - `.vscode` folder contains developer specific settings and not committed to the project.
- QT Debug Symbols
    - Needed for QT based project debugging. Without them debug will also work, but QT types cannot be inspected.
    - Not included in offline QT installer https://bugreports.qt.io/browse/QTBUG-72733
    - Should be manually downloaded and installed
        - https://download.qt.io/online/qtsdkrepository/windows_x86/desktop/qt5_51210/qt.qt5.51210.debug_info.win64_msvc2017_64/
        - QT Base is enough for beginning. https://download.qt.io/online/qtsdkrepository/windows_x86/desktop/qt5_51210/qt.qt5.51210.debug_info.win64_msvc2017_64/5.12.10-0-202011040843qtbase-Windows-Windows_10-MSVC2017-Windows-Windows_10-X86_64-debug-symbols.7z
    - Unpack over existing QT install into C:\Qt\Qt5.12.10\5.12.10\msvc2017_64\
    - Set in *launch.json* `symbolSearchPath": "C:/Qt/Qt5.12.10/5.12.10/msvc2017_64",`
    - Installing QT sources not necessary.
- QT5 Natvis
    - Natvis is a Microsoft debugger visualization framework of MS VisualStudio and MS VS Code, built into [MIEngine](https://github.com/microsoft/MIEngine)
        - https://code.visualstudio.com/docs/cpp/natvis
        - https://docs.microsoft.com/en-us/visualstudio/debugger/create-custom-views-of-native-objects?view=vs-2019
        - https://devblogs.microsoft.com/cppblog/using-visual-studio-2013-to-write-maintainable-native-visualizations-natvis/
    - Used for QT based project debugging in VS Code
        - https://wiki.qt.io/IDE_Debug_Helpers
    - For using Natvis, debugger type must be `cppvsdbg` in *launch.json*.
    - Natvis XML is used to describe types visualization.
    - QT project provides incomplete Natvis XML
        - https://github.com/qt-labs/vstools/blob/dev/QtVsTools.Package/qt5.natvis.xml
        - https://code.qt.io/cgit/qt-labs/vstools.git/plain/QtVsTools.Package/qt5.natvis.xml
        - For example QDir is not included and will not be displayed properly in the debugger.
    - Natvis XML could be manually extended with needed QT objects, for example from here:
        - https://github.com/aleksey-nikolaev/natvis-collection
        - QDir https://github.com/aleksey-nikolaev/natvis-collection/blob/master/QDir.natvis
    - Natvis XML is loaded into debugger by "Qt Visual Studio Code Tools" extension.
        - More details at https://github.com/tonka3000/vscode-qt-tools
    - Natvis XML must be manually provided to "QT Tools" extension
        - Open `Settings`, change to `Workspace`
        - Type "QT", Scroll to `Qttools: Visualizer File`
        - Set file path to `qt5.natvis.xml` or provide URL `https://raw.githubusercontent.com/qt-labs/vstools/dev/QtVsTools.Package/qt5.natvis.xml`
        - You can also change *settings.json* `qttools.visualizerFile` - same effect.
        - Do not change *launch.json* `visualizerFile`, as it is always overwritten by "QT Tools" extension.
    - Press `Ctrl+~`, choose `OUTPUT` tab and select QT on the right dropdown. This will display "QT Tools" extension log.


# Development with Visual Studio Code
## Official documentation
- https://code.visualstudio.com/docs/cpp/config-msvc
- https://code.visualstudio.com/docs/cpp/config-linux
- https://code.visualstudio.com/docs/cpp/config-clang-mac
- https://code.visualstudio.com/docs/cpp/cpp-debug
- https://code.visualstudio.com/docs/cpp/launch-json-reference

## Building, Running, Debugging
- Project uses [GIT Submodules](https://git-scm.com/book/en/v2/Git-Tools-Submodules).
    - `git submodule update --init`
- Open Command Palette: `Ctrl + Shift + P` and type...
    - `CMake: Scan for Kits`
    - `CMake: Select Kit`
        - Visual Studio Build Tools 2017 Release - amd64
    - `CMake: Select Variant`
        - Debug
    - `CMake: Set Build Target`
        - all META
    - `CMake: Clean Rebuild`
- F7 to build.
- F5 to start debugging.

## Defining CMake variables
- Edit `.vscode\settings.json`
```json
    "cmake.configureSettings": {
        "DLT_PARSER": false,
        "QDLT_LIB_DIR": "c:\\Projects\\src\\dlt-viewer\\build\\Release\\dlt-viewer\\qdlt",
        "QDLT_INCLUDE_DIR" "c:\\Projects\\src\\dlt-viewer\\dlt-viewer\\qdlt"
    },
```
# Command line build
- Windows Start Menu / Visual Studio 2017 / x64 Native Tools Command Prompt.
- OR run `"c:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\VC\Auxiliary\Build\vcvars64.bat"` from cmd.exe
```cmd
> cd dlt-viewer
> mkdir build && mkdir build\Release && cd build\Release
> cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..\..

# Build everything
> cmake --build .

# Build everything verbose
> cmake --build . -v

# Build everything in one thread
> cmake --build . -- -j1

# Build specific CMake target verbose
cmake --build . --target dltsomeipplugin -v

# To change CMake variables, delete all files in Release folder to clear CMake cache.
> rmdir /s /q ..\Release
# It will complain, but do the job - "The process cannot access the file because it is being used by another process."
> cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DDLT_PARSER=OFF ..\..

# Generate DLTViewer folder
> cmake install . --prefix C:\DltViewerSDK
```
# CMake configuration
## CMake variables
- CMake projects should be configured by CMake variables, not by environment variables!
- Environment variables cannot be set by CMake. CMake configuration passed down to build toolchain through generator. For Ninja it is *.ninja files.
- CMake variables are often cached. Many cached variables cannot be changed without deleting CMake cache! So whole `build\Release` folder should be cleaned before setting them again on command line.
- Environment variables for build toolchain should be configured **before running CMake**. See `vcvars64.bat` call above. [Use the Microsoft C++ toolset from the command line](https://docs.microsoft.com/en-us/cpp/build/building-on-the-command-line?view=msvc-170&viewFallbackFrom=msvc-141).
- `Ctrl + Shift + P`
    - CMake: Edit CMake cache
    - CMake: Edit CMake cache (UI)
- dlt-viewer
    - `option(DLT_PARSER "Build DLT Parser" OFF)`
    - `option(DLT_DUMMY_PLUGINS "Build Dummy plugins" OFF)`
    - Linux only:
        -  `option(DLT_USE_STANDARD_INSTALLATION_LOCATION "Use standard GNU installation locations" OFF)`
        - `set(DLT_PLUGIN_INSTALLATION_PATH ${CMAKE_INSTALL_FULL_LIBDIR}/dlt-viewer/plugins)`
        - `set(DLT_RESOURCE_INSTALLATION_PATH ${CMAKE_INSTALL_FULL_DATADIR}/dlt-viewer)`
        - `set(DLT_EXECUTABLE_INSTALLATION_PATH ${CMAKE_INSTALL_FULL_BINDIR})`
        - `set(DLT_LIBRARY_INSTALLATION_PATH ${CMAKE_INSTALL_FULL_LIBDIR})`
        - `option(DLT_USE_QT_RPATH "Use RPATH for QT_LIBRARY_PATH to support non-standard QT install locations" ON)`
## CMake targets
- See "CMake: Project outline" on the left in VS Code
- Verbose list of all targets `cmake --build . --target help`
    - Useful: `cmake --build . --target help | %LOCALAPPDATA%\Programs\Git\usr\bin\grep "dll\|exe"`
- Useful targets are:
    - `dlt-viewer` project
        - `dlt-viewer`, `dlt-parser`, `qdlt`,
## Build release package
- Change project version
    - Update `dlt-viewer/src/version.h`
    - version.h is parsed programmatically, do not change anything else in it.
    - Delete CMake cache and rebuild.
- 7Zip
    - I https://www.7-zip.org/download.html
- NSIS: Nullsoft Scriptable Install System
    - https://sourceforge.net/projects/nsis/
        - Click Download and wait 5sec
    - https://nsis.sourceforge.io/Download
    - `makensis` should be in the path.
```cmd
> cd dlt-viewer\build\Release
> cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..\..
> cmake --build .

> cpack -G NSIS -v
> cpack -G 7Z -v
```
## CMake install
- `cmake install . --prefix C:\DltViewerSDK` command replaces old approach with `.bat` files to prepare DLT Viewer distribution.
- Each CMake target is configured with additional files, which belongs to it. This allows to reduce amount of scripts and simplify maintenance of cross platform development.
## CPack
- [CPack](https://cmake.org/cmake/help/latest/module/CPack.html) is a separate command line utility, part of CMake.
- CPack uses same concept of generators to produce specific build artifact.
- CPack uses `cmake install` underneath.
- NSIS generator template is copied from CMake installation to the project, just to solve issue with `RequestExecutionLevel user` setting.

# Packaging
- Modern application installation approach in industry is to ship independent application and do not integrate into existing OS.
- That means application ship all libraries within a package and fully container in one folder after installation.
- On Windows `%LOCALAPPDATA%\Programs\` location is used to allow installation without Administrator rights.
- This project uses ``%LOCALAPPDATA%\Programs\DLTViewer\` path.

# "Build from sources" use-case
- There is no good use-case to use `cmake install` directly.
- To build from sources and use DLT Viewer on your machine, please use CPack to generate a package. Then just install it.

```
> cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DQDLT_LIB_DIR=c:\Projects\src\dlt-viewer\build\dlt-viewer\qdlt -DQDLT_INCLUDE_DIR=c:\Projects\src\dlt-viewer\dlt-viewer\qdlt ..\..
```
# Contribute to DLT Viewer
- Open `dlt-viewer` project
- Do CMake: Configure
- Set `dlt-viewer` as build and debug target
    - `Ctrl + Shift + P`
        - CMake: Set Build Target
            - dlt-viewer EXECUTABLE
        - CMake: Set Debug Target
            - dlt-viewer
            - Command is available when there is more than one executable in the project. So only, when this CMake variable is set `DLT_PARSER=ON`.
# DLT Viewer CI builds
- `dlt-viewer` is designed to build DLT Viewer and produce artifacts for Windows, Linux and MacOS.
- CMake configuration support both, comfortable development environment and CI builds without modifications and additional scripts.
# Microsoft Visual Studio 2019
- Development with Visual Studio is also supported by current CMake configuration starting from version VS 2019.
- There is **no need** to use CMake generator: [Visual Studio generator](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html#visual-studio-generators) and generate `.sln` and `.vcproj` files anymore. NO need to use `msbuild` and `nmake`. CMake and Ninja are fully supported by VS 2019.
- `dlt-viewer` folder should be open with command Open Directory and **not** wth Open Solution or Open Project.
- One reason to use Visual Studio could be a great built-in *C++ Profiler*. https://docs.microsoft.com/en-us/visualstudio/profiling/profiling-feature-tour?view=vs-2022

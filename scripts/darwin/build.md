# CMake + MS Visual Studio Code
Please see also [DLT Viewer: CMake + MS Visual Studio Code](../windows/build.md)

# macOs development environment setup
For DLT Viewer development on **MS Windows** please see [Windows development environment setup](../windows/build.md)
For DLT Viewer development on **Linux** please see [Linux development environment setup](../linux/build.md)

Configuration:
- macOS Catalina 10.15+
- Administrator rights `sudo`
- Build environment [install.sh](./install.sh)
    - `sudo dlt-viewer/scripts/linux/install.sh`
- MS Visual Studio Code
    - https://code.visualstudio.com/docs/setup/mac
- Setup `.vscode` folder
    - Copy content of `dlt-viewer/scripts/windows/.vscode_example/*.json` into `dlt-viewer/.vscode/`
    - `.vscode` folder contains developer specific settings and not committed to the project.

# Development with Visual Studio Code
- Please see [windows/build.md](../windows/build.md) on how to use CMake and VS Code with DLT Viewer project.

# "Build from sources" use-case
- Build Release configuration
    - [windows/build.md](../windows/build.md#command-line-build)
- Do NOT use `cmake --install .` for system-wide install.
- Do `cpack -G External` to generate macOS Application bundle.
- Copy/drag DltViewer.app to Applications folder.

# Build release package
- Same as "Build from sources" use-case.
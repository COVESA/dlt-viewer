# CMake + MS Visual Studio Code
Please see also [DLT Viewer: CMake + MS Visual Studio Code](../windows/build.md)

# Linux development environment setup
For DLT Viewer development on **MS Windows** please see [Windows development environment setup](../windows/build.md)

Configuration:
- Modern desktop Linux, Ubuntu LTS is recommended.
- Administrator rights `sudo`
- Build environment [install.sh](./install.sh)
    - `sudo dlt-viewer/scripts/linux/install.sh`
- QT 5.12.10
    - Default QT version published with each Ubuntu version is different. Good one is installed by `install.sh`
    - Source QT5.12.10 environment. **Don't miss dot**.
        - `. /opt/qt512/bin/qt512-env.sh`
    - OR use `-DCMAKE_PREFIX_PATH=/opt/qt512/lib/cmake`
- MS VS Code
```bash
curl -sSL https://packages.microsoft.com/keys/microsoft.asc | sudo apt-key add -
sudo add-apt-repository "deb [arch=amd64] https://packages.microsoft.com/repos/vscode stable main"
sudo apt update
sudo apt install code
```
- Ubuntu note
    - Do not go to *Snap Store*.
    - Do not install VS Code from Ubuntu App Gallery. This will provide [Flatpak](https://docs.flatpak.org/en/latest/sandbox-permissions.html) version of VS Code, which will run in the container and will have [limited access to the filesystem](https://stackoverflow.com/questions/55930071/vscode-cant-find-files-in-linux-system).
- Setup `.vscode` folder
    - Copy content of `dlt-viewer/scripts/linux/.vscode_example/*.json` into `dlt-viewer/.vscode/`
    - `.vscode` folder contains developer specific settings and not committed to the project.

# Development with Visual Studio Code
- Please see [windows/build.md](../windows/build.md) on how to use CMake and VS Code with DLT Viewer project.

# "Build from sources" use-case
- Build Release configuration and do `cmake --install .` for system-wide install, or `cmake --install . --prefix DLTViewer` for local install.
    - Set CMake variable `DLT_USE_STANDARD_INSTALLATION_LOCATION=ON`, see [CMakeLists.txt](../CMakeLists.txt)
    - OR set `DLT_USE_STANDARD_INSTALLATION_LOCATION=OFF` and use `DLT_XXX_INSTALLATION_PATH` variables, see [build.sh](./build.sh)

# Build release package
- DLT Viewer for Linux is distributed as AppImage.
    - See [install.md](./install.md) for details.
    - AppImage approach is analogues to MacOS `DMG` format.
    - To generate proper AppImage all project dependencies should be collected in `AppDir/usr/lib` folder and binaries should be patched with relative `DT_RUNPATHL`, for example `$ORIGIN/../lib`.
    - [linuxdeploy](https://github.com/linuxdeploy/linuxdeploy) is used for this purpose with [linuxdeploy-plugin-qt](https://github.com/linuxdeploy/linuxdeploy-plugin-qt).
        - https://docs.appimage.org/reference/best-practices.html#general-recommendations
        - DT_RUNPATH documentation https://linux.die.net/man/1/ld

- `linuxdeploy` utility is wrapped into CPack External generator.
- [build.sh](./build.sh) script generates DLT Viewer AppImage and also produces TGZ file.
- Please use oldest supported Ubuntu LTS, currently Ubuntu 18.04, to create AppImage package compatible with most of desktop Linux installations.
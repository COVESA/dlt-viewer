# Installation of DLT Viewer on Linux
# Requirements
- Supported platforms
    - Ubuntu 18.04 x86_64 and later.
    - Most of modern desktop Linux distributions with **glibc >= 2.27**.
        - `ldd --version`
- Administrator rights `sudo` are **not needed**
- No need to download and install C++ build tools, QT framework **or any other dependencies**, they are included in AppImage.

# AppImage
- AppImage files are simpler than installing an application. No extraction tools are needed, nor is it necessary to modify the operating system or user environment. Regular users on the common Linux distributions can download it, make it executable, and run it. [Wikipedia](https://en.wikipedia.org/wiki/AppImage)
- [AppImage - Linux apps that run anywhere](https://appimage.org/)
- https://github.com/AppImage/AppImageKit

# Installation
- https://docs.appimage.org/user-guide/faq.html
- Download and unpack DLT Viewer archive.
    - Copy AppImage into `~/bin/`
- Make it executable and run
    - `chmod +x ~/bin/DLT_Viewer-2.21.3-x86_64.AppImage`
    - https://discourse.appimage.org/t/how-to-run-an-appimage/80
    - https://docs.appimage.org/user-guide/run-appimages.html
- Command line mode of DLT Viewer also works in terminal
    - `./DLT_Viewer-2.21.3-x86_64.AppImage --help`

## Troubleshooting
- AppImage on start creates filesystem mount to `/tmp` using user space FUSE driver. If FUSE is not available, AppImage can be instructed to simply unpack into `/tmp`
    - `./DLT_Viewer-2.21.3-x86_64.AppImage --appimage-extract-and-run`
    - https://docs.appimage.org/user-guide/troubleshooting/index.html
    - https://docs.appimage.org/user-guide/troubleshooting/fuse.html#fallback-if-fuse-can-t-be-made-working
- If AppImage dose not start - try to run it from console. Right click in file manager, *Open Terminal here*.
- When AppImage is mounted, its content could be accessed and [AppDir](https://docs.appimage.org/reference/appdir.html#ref-appdir) could be copied to any location.
    - This command can mount AppImage `~/bin/DLT_Viewer-2.21.3-x86_64.AppImage --appimage-mount`
- AppImage help
    - `~/bin/DLT_Viewer-2.21.3-x86_64.AppImage --appimage-help`

## Optional desktop integration
### Automatic integration
- Modern Linux distributions, like Manjaro, already include AppImageLauncher. This will display dialog to integrate AppImage into desktop automatically.
- Also there is a daemon [appimaged](https://github.com/probonopd/go-appimage), which can take care of AppImage.

### Manual integration
- Copy AppImage into `~/bin` and make it executable
    - `chmod +x ~/bin/DLT_Viewer-2.21.3-x86_64.AppImage`
- Mount AppImage
    - `~/bin/DLT_Viewer-2.21.3-x86_64.AppImage --appimage-mount`
    - Note printed out temporary path `/tmp/.mount_DLT_ViWVylOY`
- Open new terminal window
- Copy `.desktop` file and icon
    - `cp /tmp/.mount_DLT_ViWVylOY/org.genivi.DLTViewer.desktop ~/.local/share/applications`
    - `chmod +x ~/.local/share/applications/org.genivi.DLTViewer.desktop`
    - `cp /tmp/.mount_DLT_ViWVylOY/org.genivi.DLTViewer.png ~/bin`
- Edit `org.genivi.DLTViewer.desktop` with `nano` or `vi`
    - `nano ~/.local/share/applications/org.genivi.DLTViewer.desktop`
        - `Exec=/home/<user name>/bin/DLT_Viewer-2.21.3-x86_64.AppImage`
        - `ICON=/home/<user name>/bin/org.genivi.DLTViewer.png`
    - Full paths are mandatory, `~` as home path **will not work**.
- DLT Viewer will appear in *Applications* menu automatically.

# Uninstall
- Delete AppImage
    - `rm ~/bin/DLT_Viewer-2.21.3-x86_64.AppImage`
- Delete desktop integration
    - `rm ~/.local/share/applications/org.genivi.DLTViewer.desktop`
    - `rm ~/bin/org.genivi.DLTViewer.png`

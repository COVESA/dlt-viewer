# Installation of DLT Viewer on Windows
# Requirements
- Supported platforms
    - Windows 7 x86_64
    - Windows 10 x86_64
    - Windows 11 x86_64
- Administrator rights are **not needed**
- No need to download and install QT framework.

# Installation
- Install **Microsoft Visual C++ 2017 Redistributable (x64)** or later
        - Download **latest package** from [Visual C++ Redistributable](https://support.microsoft.com/en-us/help/2977003/)
        - For example https://aka.ms/vs/17/release/vc_redist.x64.exe
        - Redistributable provides backward binary compatibility https://docs.microsoft.com/en-us/cpp/porting/binary-compat-2015-2017?view=msvc-170
- Go to [Releases](https://github.com/COVESA/dlt-viewer/releases) section
    - Installer executable
        - Installer default installation directory does not require Administrator rights.
            - `c:\Users\%USER%\AppData\Local\Programs\DLTViewer`
        - Installer creates Windows Start Menu shortcuts to DLT Viewer and documentation.
        - Everything is installed into one location
    - 7zip archive
        - Unpack to any folder and just run dlt-viewer.exe from there.

# Uninstallation
- Installer executable.
    - Go to Control Panel / Programs and Features.
    - Select DLTViewer from the list.
- 7zip archive.
    - Just delete unpacked folder.
- Clean DLT Viewer cache.
    - Delete folder `c:\Users\%USER%\AppData\Local\dlt-viewer`

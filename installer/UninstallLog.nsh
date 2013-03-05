;AddItem macro
  !macro AddItem Path
    FileWrite $UninstLog "${Path}$\r$\n"
  !macroend
 
  ;File macro
  !macro File InstallPath FileName WorkDir 
     ;IfFileExists "$INSTDIR\${FileName}" +2
     FileWrite $UninstLog "${InstallPath}\${FileName}$\r$\n"
	 DetailPrint ${WorkDir}
	 File /oname=${FileName} ${WorkDir}\${FileName}
  !macroend

  ;FileDir macro
  !macro FileDir InstallPath FileName WorkDir 
     ;IfFileExists "$INSTDIR\${FileName}" +2
     FileWrite $UninstLog "${InstallPath}\${FileName}$\r$\n"
	 File /r ${WorkDir}\*${FileName}*
  !macroend
  
  ;FileType macro
  !macro FileType InstallPath FileName WorkDir 
     FileWrite $UninstLog "${InstallPath}\*.${FileName}$\r$\n"
	 File ${WorkDir}\*.${FileName}
  !macroend
  
  
  ;CreateShortcut macro
  !macro CreateShortcut FilePath FilePointer Pamameters Icon IconIndex
    FileWrite $UninstLog "${FilePath}$\r$\n"
    CreateShortcut "${FilePath}" "${FilePointer}" "${Pamameters}" "${Icon}" "${IconIndex}"
  !macroend
 
;Copy files macro
  !macro CopyFiles SourcePath DestPath
    IfFileExists "${DestPath}" +2
    FileWrite $UninstLog "${DestPath}$\r$\n"
    CopyFiles "${SourcePath}" "${DestPath}"
  !macroend
 
;Rename macro
  !macro Rename SourcePath DestPath
    IfFileExists "${DestPath}" +2
    FileWrite $UninstLog "${DestPath}$\r$\n"
    Rename "${SourcePath}" "${DestPath}"
  !macroend
 
;CreateDirectory macro
  !macro CreateDirectory Path
    CreateDirectory "${Path}"
    FileWrite $UninstLog "${Path}$\r$\n"
  !macroend
 
;SetOutPath macro
  !macro SetOutPath Path
    SetOutPath "${Path}"
    FileWrite $UninstLog "${Path}$\r$\n"
  !macroend
 
;WriteUninstaller macro
  !macro WriteUninstaller Path
    WriteUninstaller "${Path}"
    FileWrite $UninstLog "${Path}$\r$\n"
  !macroend
 
;WriteIniStr macro
  !macro WriteIniStr IniFile SectionName EntryName NewValue
     IfFileExists "${IniFile}" +2
     FileWrite $UninstLog "${IniFile}$\r$\n"
     WriteIniStr "${IniFile}" "${SectionName}" "${EntryName}" "${NewValue}"
  !macroend
 
;WriteRegStr macro
  !macro WriteRegStr RegRoot UnInstallPath Key Value
     FileWrite $UninstLog "${RegRoot} ${UnInstallPath}$\r$\n"
     WriteRegStr "${RegRoot}" "${UnInstallPath}" "${Key}" "${Value}"
  !macroend
 
 
;WriteRegDWORD macro
  !macro WriteRegDWORD RegRoot UnInstallPath Key Value
     FileWrite $UninstLog "${RegRoot} ${UnInstallPath}$\r$\n"
     WriteRegDWORD "${RegRoot}" "${UnInstallPath}" "${Key}" "${Value}"
  !macroend
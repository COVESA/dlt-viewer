;---------------------------------
; NEW UI
;---------------------------------

!include "MUI.nsh"
!include "UninstallLog.nsh"


!Define PRODUCT_NAME "DLTViewer"
Name "${PRODUCT_NAME}"
!Define EXE_NAME "dlt_viewer.exe"
!Define PRODUCT_VERSION "2.9.1"
!Define VER_MAJOR 2
!Define VER_MINOR 9
!Define VER_REVISION 1
!Define PRODUCT_SUPPORT_SITE \
    "http://dlt.bmwgroup.net"
!Define PRODUCT_DIR_REGKEY \
    "Software\Microsoft\Windows\CurrentVersion\App Paths\${EXE_NAME}"
!Define PRODUCT_UNINST_KEY \
   "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!Define REG_KEY "SOFTWARE\$(^Name)"
!Define INSTALL_ROOT_KEY "HKLM"
!Define CONF_FILE "$INSTDIR\config.ini"
!Define WORKING_DIR "C:\DltViewerSDK"


SetPluginUnload  alwaysoff


;--------------------------------
; Configuration
;---------------------------------
Caption "${PRODUCT_NAME} ${PRODUCT_VERSION} Setup"
!Ifdef OUTFILE
OutFile "${OUTFILE}"
!Else
OutFile "${WORKING_DIR}\installer\${PRODUCT_NAME}-${PRODUCT_VERSION}-setup.exe"
!Endif
; Installer attributes
InstallDir $PROGRAMFILES\${PRODUCT_NAME}
InstallDirRegKey ${INSTALL_ROOT_KEY} "${PRODUCT_DIR_REGKEY}" ""

;---------------------------------
;Pages
;---------------------------------
;!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "${WORKING_DIR}\LICENSE.txt"
!insertmacro MUI_PAGE_DIRECTORY 
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES




;--------------------------------
; Configure UnInstall log to only remove what is installed
;-------------------------------- 
  ;Set the name of the uninstall log
    !define UninstLog "uninstall.log"
    Var UninstLog
 
  ;Uninstall log file missing.
    LangString UninstLogMissing ${LANG_ENGLISH} "${UninstLog} not found!$\r$\nUninstallation cannot proceed!"

    ;File macro
    !define File "!insertmacro File"
	!define FileDir "!insertmacro FileDir"
	!define FileType "!insertmacro FileType"
  ;CreateShortcut macro
    !define CreateShortcut "!insertmacro CreateShortcut"
 
  ;SetOutPath macro
    !define SetOutPath "!insertmacro SetOutPath"
 
  ;WriteUninstaller macro
    !define WriteUninstaller "!insertmacro WriteUninstaller"
 
  ;WriteRegStr macro
    !define WriteRegStr "!insertmacro WriteRegStr"
    
  ;CreateDirectory macro
    !define CreateDirectory "!insertmacro CreateDirectory"
 

 
 
  Section -openlogfile
    CreateDirectory "$INSTDIR"
    IfFileExists "$INSTDIR\${UninstLog}" +3
      FileOpen $UninstLog "$INSTDIR\${UninstLog}" w
    Goto +4
      SetFileAttributes "$INSTDIR\${UninstLog}" NORMAL
      FileOpen $UninstLog "$INSTDIR\${UninstLog}" a
      FileSeek $UninstLog 0 END
  SectionEnd

;--------------------------------
; Installer Sections
;--------------------------------
Section "DLT Viewer" secmain
;SetDetailsPrint extOnly \
;detailed print "Installing ${PRODUCT_NAME} core ${File}s ..."
SetDetailsPrint listonly
SectionIn 1 2 3 RO
SetShellVarContext all
SetOutPath $INSTDIR
SetOverwrite on
${File} $INSTDIR dlt_viewer.exe ${WORKING_DIR}
${File} $INSTDIR LICENSE.txt ${WORKING_DIR}
${File} $INSTDIR MPL.txt ${WORKING_DIR}
${File} $INSTDIR README.txt ${WORKING_DIR}
${FileType} $INSTDIR dll ${WORKING_DIR}
;${File} $INSTDIR *.dll ${WORKING_DIR}
;${File} $INSTDIR libgcc_s_dw2-1.dll ${WORKING_DIR}
;${File} $INSTDIR libstdc++-6.dll ${WORKING_DIR}
;${File} $INSTDIR mingwm10.dll ${WORKING_DIR}
;${File} $INSTDIR MOSTDecoder.dll ${WORKING_DIR}
;${File} $INSTDIR qdlt.dll ${WORKING_DIR}
;${File} $INSTDIR qextserialport.dll ${WORKING_DIR}
;${File} $INSTDIR QtCore4.dll ${WORKING_DIR}
;${File} $INSTDIR QtGui4.dll ${WORKING_DIR}
;${File} $INSTDIR QtNetwork4.dll ${WORKING_DIR}
;${File} $INSTDIR QtSql4.dll ${WORKING_DIR}
;${File} $INSTDIR QtXml4.dll ${WORKING_DIR}
;${File} $INSTDIR qwt.dll ${WORKING_DIR}

${File} $INSTDIR ReleaseNotes_Viewer.txt ${WORKING_DIR}

;Create uninstaller
  ${WriteUninstaller} "$INSTDIR\Uninstall.exe"

MessageBox MB_YESNO|MB_ICONQUESTION "Do you wish to create start-menu shortcuts for ${PRODUCT_NAME}?" IDNO NoShortcuts
${CreateDirectory} "$SMPROGRAMS\${PRODUCT_NAME}"
${CreateShortCut} "$SMPROGRAMS\${PRODUCT_NAME}\Uninstall DLT Viewer.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
${CreateShortCut} "$SMPROGRAMS\${PRODUCT_NAME}\${PRODUCT_NAME}.lnk" "$INSTDIR\${EXE_NAME}" "" "$INSTDIR\${EXE_NAME}" 0
NoShortcuts:


SectionEnd

;--------------------------------
;Plugin Section 
;--------------------------------
Section "Plugins" secplug
    SetDetailsPrint listonly
    SectionIn 1 2 3 
    SetShellVarContext all
    SetOutPath $INSTDIR
    SetOverwrite on
    ${FileDir} $INSTDIR plugins ${WORKING_DIR}
SectionEnd

;--------------------------------
;Filters Section
;--------------------------------
Section "Filters" secfilt
    SetDetailsPrint listonly
    SectionIn 1 2 3
    SetShellVarContext all
    SetOutPath $INSTDIR
    SetOverwrite on
    ${FileDir} $INSTDIR filters ${WORKING_DIR}
SectionEnd

;--------------------------------
;SDK Section
;--------------------------------
Section "SDK" secsdk
    SetDetailsPrint listonly
    SectionIn 1
    SetShellVarContext all
    SetOutPath $INSTDIR
    SetOverwrite on
    ${FileDir} $INSTDIR sdk ${WORKING_DIR}
SectionEnd


;--------------------------------
;Documentation Section
;--------------------------------
Section "Documentation" secdoc
    SetDetailsPrint listonly
    SectionIn 1
    SetShellVarContext all
    SetOutPath $INSTDIR
    SetOverwrite on
    ${FileDir} $INSTDIR doc ${WORKING_DIR}
SectionEnd

;--------------------------------
;Sqldrivers Section
;--------------------------------
Section "Sqldrivers" secsql
    SetDetailsPrint listonly
    SectionIn 1
    SetShellVarContext all
    SetOutPath $INSTDIR
    SetOverwrite on
    ${FileDir} $INSTDIR sqldrivers ${WORKING_DIR}
SectionEnd

;--------------------------------
;Platforms Section
;--------------------------------
Section "Platforms" secpla
    SetDetailsPrint listonly
    SectionIn 1
    SetShellVarContext all
    SetOutPath $INSTDIR
    SetOverwrite on
    ${FileDir} $INSTDIR platforms ${WORKING_DIR}
SectionEnd

;--------------------------------
;Cache Section
;--------------------------------
Section "Cache" seccac
    SetDetailsPrint listonly
    SectionIn 1
    SetShellVarContext all
    SetOutPath $INSTDIR
    SetOverwrite on
    ${FileDir} $INSTDIR cache ${WORKING_DIR}
SectionEnd

;--------------------------------
;Uninstaller Section
;--------------------------------
;--------------------------------
; Uninstaller
;--------------------------------
Section Uninstall
  ;Can't uninstall if uninstall log is missing!
  IfFileExists "$INSTDIR\${UninstLog}" +3
    MessageBox MB_OK|MB_ICONSTOP "$(UninstLogMissing)"
      Abort
 
  Push $R0
  Push $R1
  Push $R2
  SetFileAttributes "$INSTDIR\${UninstLog}" NORMAL
  FileOpen $UninstLog "$INSTDIR\${UninstLog}" r
  StrCpy $R1 -1
 
  GetLineCount:
    ClearErrors
    FileRead $UninstLog $R0
    IntOp $R1 $R1 + 1
    StrCpy $R0 $R0 -2
    Push $R0   
    IfErrors 0 GetLineCount
 
  Pop $R0
 
  LoopRead:
    StrCmp $R1 0 LoopDone
    Pop $R0
 
    IfFileExists "$R0\*.*" 0 +3
      RMDir /r $R0  #is dir
    Goto +9
    IfFileExists $R0 0 +3
      Delete $R0 #is file
    Goto +6
    StrCmp $R0 "${INSTALL_ROOT_KEY} ${PRODUCT_DIR_REGKEY}" 0 +3
      DeleteRegKey "${INSTALL_ROOT_KEY}" "${PRODUCT_DIR_REGKEY}" #is Reg Element
    Goto +3
    StrCmp $R0 "${INSTALL_ROOT_KEY} ${PRODUCT_DIR_REGKEY}" 0 +2
      DeleteRegKey "${INSTALL_ROOT_KEY}" "${PRODUCT_DIR_REGKEY}" #is Reg Element
 
    IntOp $R1 $R1 - 1
    Goto LoopRead
  LoopDone:
  FileClose $UninstLog
  Delete "$INSTDIR\${UninstLog}"
  
  
  
  IfFileExists "${CONF_FILE}" 0 +3
    MessageBox MB_YESNO|MB_ICONQUESTION "Do you want to delete the personal configuration?" IDNO NOCONFIGDEL
        Delete "${CONF_FILE}"
    NOCONFIGDEL:
        
        
  RMDir "$INSTDIR"
  Pop $R2
  Pop $R1
  Pop $R0
 
  ;Remove registry keys
    ;DeleteRegKey ${REG_ROOT} "${REG_APP_PATH}"
    ;DeleteRegKey ${REG_ROOT} "${UNINSTALL_PATH}"
SectionEnd



;LANGSTRINGS:
LangString "MUI_TEXT_LICENSE_TITLE" ${LANG_ENGLISH} "DLT Viewer License"
LangString "MUI_INNERTEXT_LICENSE_BOTTOM"  ${LANG_ENGLISH} "Please agree to the license agreement"
LangString "MUI_TEXT_LICENSE_SUBTITLE"  ${LANG_ENGLISH} ""
LangString "MUI_INNERTEXT_LICENSE_TOP"  ${LANG_ENGLISH} ""
LangString "MUI_TEXT_INSTALLING_TITLE"  ${LANG_ENGLISH} "DLT Viewer"
LangString "MUI_TEXT_INSTALLING_SUBTITLE"  ${LANG_ENGLISH} "Now the files are being copied to your machine."
LangString "MUI_TEXT_FINISH_TITLE"  ${LANG_ENGLISH} "Installation complete!"
LangString "MUI_TEXT_FINISH_SUBTITLE"  ${LANG_ENGLISH} "Now you can close the window."
LangString "MUI_TEXT_ABORT_TITLE"  ${LANG_ENGLISH} "--- N/A ----"
LangString "MUI_TEXT_ABORT_SUBTITLE" ${LANG_ENGLISH} "--- N/A ----"
LangString "MUI_TEXT_DIRECTORY_TITLE" ${LANG_ENGLISH} "Choose the install directory"
LangString "MUI_TEXT_DIRECTORY_SUBTITLE" ${LANG_ENGLISH} "Where do you want to install ${PRODUCT_NAME} ${PRODUCT_VERSION}"
LangString "MUI_WELCOMEPAGE_TITLE" ${LANG_ENGLISH} "${PRODUCT_NAME} ${PRODUCT_VERSION}"
LangString "MUI_WELCOMEPAGE_TEXT " ${LANG_ENGLISH} "Follow the instructions to install the DLT Viewer ${PRODUCT_NAME} ${PRODUCT_VERSION}"
LangString "MUI_TEXT_COMPONENTS_TITLE"  ${LANG_ENGLISH} "${PRODUCT_NAME} components"
LangString "MUI_UNCONFIRMPAGE_TEXT_TOP"  ${LANG_ENGLISH} "Uninstaller: ${PRODUCT_NAME} ${PRODUCT_VERSION}"
LangString "MUI_UNTEXT_CONFIRM_SUBTITLE"  ${LANG_ENGLISH} ""
LangString "MUI_UNTEXT_CONFIRM_TITLE"  ${LANG_ENGLISH} "Uninstall ${PRODUCT_NAME} ${PRODUCT_VERSION}"
LangString "MUI_UNTEXT_UNINSTALLING_TITLE"  ${LANG_ENGLISH} "Uninstalling ${PRODUCT_NAME} ${PRODUCT_VERSION}..."
LangString "MUI_UNTEXT_FINISH_TITLE"  ${LANG_ENGLISH} "${PRODUCT_NAME} ${PRODUCT_VERSION} has been removed from your system."

 




  

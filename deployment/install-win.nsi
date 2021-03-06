; Script generated by the HM NIS Edit Script Wizard.

; HM NIS Edit Wizard helper defines
!define PRODUCT_NAME "irrlamb"
!define PRODUCT_VERSION "0.2.0"
!define PRODUCT_PUBLISHER "Alan Witkowski"
!define PRODUCT_WEB_SITE "https://github.com/jazztickets/irrlamb"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\irrlamb.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"

; MUI 1.67 compatible ------
!include "MUI.nsh"

; MUI Settings
!define MUI_ABORTWARNING
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

; Welcome page
!insertmacro MUI_PAGE_WELCOME
; License page
!insertmacro MUI_PAGE_LICENSE "license.txt"
; Directory page
!insertmacro MUI_PAGE_DIRECTORY
; Instfiles page
!insertmacro MUI_PAGE_INSTFILES
; Finish page
!define MUI_FINISHPAGE_RUN "$INSTDIR\irrlamb.exe"
; !define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\readme.txt"
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
!insertmacro MUI_UNPAGE_INSTFILES

; Language files
!insertmacro MUI_LANGUAGE "English"

; MUI end ------

RequestExecutionLevel admin
Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "irrlamb-${PRODUCT_VERSION}-setup.exe"
InstallDir "$PROGRAMFILES\irrlamb"
InstallDirRegKey HKLM "${PRODUCT_DIR_REGKEY}" ""
ShowInstDetails show
ShowUnInstDetails show

Section "MainSection" SEC01
  SetShellVarContext all
  SetOutPath "$INSTDIR"
  File "..\CHANGELOG"
  File "..\LICENSE"
  File "..\README"
  File "..\bin\Release\irrlamb.exe"
  CreateDirectory "$SMPROGRAMS\irrlamb"
  CreateShortCut "$SMPROGRAMS\irrlamb\irrlamb.lnk" "$INSTDIR\irrlamb.exe"
  CreateShortCut "$DESKTOP\irrlamb.lnk" "$INSTDIR\irrlamb.exe"
  File "..\working\Irrlicht.dll"
  File "..\working\OpenAL32.dll"
  File /r "..\working\art"
  File /r "..\working\fonts"
  File /r "..\working\levels"
  File /r "..\working\meshes"
  File /r "..\working\scripts"
  File /r "..\working\shaders"
  File /r "..\working\sounds"
  File /r "..\working\textures"
SectionEnd

Section -AdditionalIcons
  SetShellVarContext all
  WriteIniStr "$INSTDIR\${PRODUCT_NAME}.url" "InternetShortcut" "URL" "${PRODUCT_WEB_SITE}"
  CreateShortCut "$SMPROGRAMS\irrlamb\Website.lnk" "$INSTDIR\${PRODUCT_NAME}.url"
  CreateShortCut "$SMPROGRAMS\irrlamb\Uninstall.lnk" "$INSTDIR\uninst.exe"
SectionEnd

Section -Post
  WriteUninstaller "$INSTDIR\uninst.exe"
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\irrlamb.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\irrlamb.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
SectionEnd


Function un.onUninstSuccess
  HideWindow
  MessageBox MB_ICONINFORMATION|MB_OK "$(^Name) was successfully removed from your computer."
FunctionEnd

Function un.onInit
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "Are you sure you want to completely remove $(^Name) and all of its components?" IDYES +2
  Abort
FunctionEnd

Section Uninstall
  SetShellVarContext all
  Delete "$INSTDIR\${PRODUCT_NAME}.url"
  Delete "$INSTDIR\uninst.exe"
  Delete "$INSTDIR\Irrlicht.dll"
  Delete "$INSTDIR\OpenAL32.dll"
  Delete "$INSTDIR\irrlamb.exe"
  Delete "$INSTDIR\LICENSE"
  Delete "$INSTDIR\CHANGELOG"
  Delete "$INSTDIR\README"
  
  Delete "$SMPROGRAMS\irrlamb\Uninstall.lnk"
  Delete "$SMPROGRAMS\irrlamb\Website.lnk"
  Delete "$DESKTOP\irrlamb.lnk"
  Delete "$SMPROGRAMS\irrlamb\irrlamb.lnk"
  
  RMDir "$SMPROGRAMS\irrlamb"
  RMDir /r "$INSTDIR\art"
  RMDir /r "$INSTDIR\fonts"
  RMDir /r "$INSTDIR\levels"
  RMDir /r "$INSTDIR\meshes"
  RMDir /r "$INSTDIR\scripts"
  RMDir /r "$INSTDIR\shaders"
  RMDir /r "$INSTDIR\sounds"
  RMDir /r "$INSTDIR\textures"
  
  RMDir "$INSTDIR"
  
  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"
  SetAutoClose true
SectionEnd

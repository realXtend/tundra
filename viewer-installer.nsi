!define VERSION "0.2.5-rc3"

Name "Naali ${VERSION}"

Page directory
Page instfiles

InstallDir "$PROGRAMFILES\Naali${VERSION}"
VIProductVersion "${VERSION}"

OutFile "Naali-0.2.5-rc3.exe"

XPStyle on

RequestExecutionLevel admin

Section ""
  SetOutPath $INSTDIR
  File /r build\*.*

  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Naali" \
                   "DisplayName" "Naali"

  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Naali" \
                   "DisplayVersion" "${VERSION}"

  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Naali" \
                   "UninstallString" "$INSTDIR\uninstaller.exe"

  ExecWait '"$INSTDIR\oalinst.exe"'
  ExecWait '"$INSTDIR\vcredist_x86.exe" /q'
  ExecWait '"$INSTDIR\dxwebsetup.exe"'

  WriteUninstaller "$INSTDIR\uninstaller.exe"

  Delete "$INSTDIR\vcredist_x86.exe"
  Delete "$INSTDIR\oalinst.exe"
  Delete "$INSTDIR\dxwebsetup.exe"
SectionEnd

Section "Start Menu Shortcuts"
  SetShellVarContext all
  CreateDirectory "$SMPROGRAMS\Naali ${VERSION}"
  CreateShortCut "$SMPROGRAMS\Naali ${VERSION}\Naali ${VERSION}.lnk" "$INSTDIR\viewer.exe"
  CreateShortCut "$SMPROGRAMS\Naali ${VERSION}\Readme.lnk" "$INSTDIR\readme.txt"
  CreateShortCut "$SMPROGRAMS\Naali ${VERSION}\Uninstall.lnk" "$INSTDIR\uninstaller.exe"
SectionEnd

Section "Uninstall"
  RMDir /r $INSTDIR
  
  #removing the APPDATA\roaming\realxtend doesn't seem to work properly on Vista. (maybe incorrect user information is given through add/remove programs)
  SetShellVarContext all
  RMDir /r "$APPDATA\Roaming\realXtend"
  RMDir /r "$SMPROGRAMS\Naali ${VERSION}"

  SetShellVarContext current
  RMDir /r "$APPDATA\Roaming\realXtend"
  RMDir /r "$SMPROGRAMS\Naali ${VERSION}"

  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Naali"
SectionEnd

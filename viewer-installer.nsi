Name "Naali"

Page directory
Page instfiles

InstallDir "$PROGRAMFILES\Naali"
VIProductVersion "0.0.2"

OutFile "naali-viewer-installer.exe"

XPStyle on

RequestExecutionLevel admin

Section ""
  SetOutPath $INSTDIR
  File /r build\*.*

  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Naali" \
                   "DisplayName" "Naali"

  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Naali" \
                   "DisplayVersion" "0.0.2"

  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Naali" \
                   "UninstallString" "$INSTDIR\uninstaller.exe"

  ExecWait '"$INSTDIR\vcredist_x86.exe" /q'

  WriteUninstaller "$INSTDIR\uninstaller.exe"

  Delete "$INSTDIR\vcredist_x86.exe"
SectionEnd

Section "Start Menu Shortcuts"
  SetShellVarContext all
  CreateDirectory "$SMPROGRAMS\Naali"
  CreateShortCut "$SMPROGRAMS\Naali\Naali.lnk" "$INSTDIR\viewer.exe"
  CreateShortCut "$SMPROGRAMS\Naali\Readme.lnk" "$INSTDIR\readme.txt"
  CreateShortCut "$SMPROGRAMS\Naali\Uninstall.lnk" "$INSTDIR\uninstaller.exe"
SectionEnd

Section "Uninstall"
  RMDir /r $INSTDIR
  
  #removing the APPDATA\roaming\realxtend doesn't seem to work properly on Vista. (maybe incorrect user information is given through add/remove programs)
  SetShellVarContext all
  RMDir /r "$APPDATA\Roaming\realXtend"
  RMDir /r "$SMPROGRAMS\Naali"

  SetShellVarContext current
  RMDir /r "$APPDATA\Roaming\realXtend"
  RMDir /r "$SMPROGRAMS\Naali"

  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Naali"
SectionEnd

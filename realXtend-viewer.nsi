Name "realXtend Naali"

Page directory
Page instfiles

InstallDir "$PROGRAMFILES\realXtend Naali"
VIProductVersion "0.0.1"

OutFile "realXtend-viewer.exe"

XPStyle on

RequestExecutionLevel admin

Section ""
  SetOutPath $INSTDIR
  File /r build\*.*

  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\realXtendNaali" \
                   "DisplayName" "realXtend Naali"

  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\realXtendNaali" \
                   "DisplayVersion" "0.0.1"

  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\realXtendNaali" \
                   "UninstallString" "$INSTDIR\uninstaller.exe"

  ExecWait '"$INSTDIR\vcredist_x86.exe" /q'

  WriteUninstaller "$INSTDIR\uninstaller.exe"

  Delete "$INSTDIR\vcredist_x86.exe"
SectionEnd

Section "Start Menu Shortcuts"
  SetShellVarContext all
  CreateDirectory "$SMPROGRAMS\realXtend Naali"
  CreateShortCut "$SMPROGRAMS\realXtend Naali\realXtend Naali.lnk" "$INSTDIR\viewer.exe"
  CreateShortCut "$SMPROGRAMS\realXtend Naali\Uninstall.lnk" "$INSTDIR\uninstaller.exe"
SectionEnd

Section "Uninstall"
  RMDir /r $INSTDIR
  
  #removing the APPDATA\roaming\realxtend doesn't seem to work properly on Vista. (maybe incorrect user information is given through add/remove programs)
  SetShellVarContext all
  RMDir /r "$APPDATA\Roaming\realXtend"
  RMDir /r "$SMPROGRAMS\realXtend Naali"

  SetShellVarContext current
  RMDir /r "$APPDATA\Roaming\realXtend"
  RMDir /r "$SMPROGRAMS\realXtend Naali"

  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\realXtendNaali"
SectionEnd
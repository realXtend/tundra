Name "realXtend-viewer"

Page directory
Page instfiles

InstallDir "$PROGRAMFILES\realXtend-viewer"
VIProductVersion "0.0.1"

OutFile "realXtend-viewer.exe"

XPStyle on

RequestExecutionLevel admin

Section ""
  SetOutPath $INSTDIR
  File /r build\*.*

  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\realXtend-viewer" \
                   "DisplayName" "realXtend-viewer"

  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\realXtend-viewer" \
                   "DisplayVersion" "0.0.1"

  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\realXtend-viewer" \
                   "UninstallString" "$INSTDIR\uninstaller.exe"

  ExecWait '"$INSTDIR\vcredist_x86.exe" /q'

  WriteUninstaller "$INSTDIR\uninstaller.exe"

  Delete "$INSTDIR\vcredist_x86.exe"
SectionEnd

Section "Start Menu Shortcuts"
  SetShellVarContext all
  CreateDirectory "$SMPROGRAMS\realXtend-viewer"
  CreateShortCut "$SMPROGRAMS\realXtend-viewer\realXtend viewer.lnk" "$INSTDIR\viewer.exe"
  CreateShortCut "$SMPROGRAMS\realXtend-viewer\Uninstall.lnk" "$INSTDIR\uninstaller.exe"
SectionEnd

Section "Uninstall"
  RMDir /r $INSTDIR
  
  #removing the APPDATA\roaming\realxtend doesn't seem to work properly on Vista. (maybe incorrect user information is given through add/remove programs)
  SetShellVarContext all
  RMDir /r "$APPDATA\Roaming\realXtend"
  RMDir /r "$SMPROGRAMS\realXtend-viewer"

  SetShellVarContext current
  RMDir /r "$APPDATA\Roaming\realXtend"
  RMDir /r "$SMPROGRAMS\realXtend-viewer"

  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\realXtend-viewer"
SectionEnd
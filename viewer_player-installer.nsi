!define VERSION "0.4.0-RC2"

Name "Naali Player ${VERSION}"

Page directory
Page instfiles

InstallDir "$PROGRAMFILES\Naali Player ${VERSION}"
VIProductVersion "${VERSION}"

OutFile "NaaliPlayer-0.4.0-RC2.exe"

XPStyle on

RequestExecutionLevel admin

Section ""
  SetOutPath $INSTDIR
  File /r buildplayer\*.*

  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\NaaliPlayer" \
                   "DisplayName" "NaaliPlayer"

  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\NaaliPlayer" \
                   "DisplayVersion" "${VERSION}"

  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\NaaliPlayer" \
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
  CreateDirectory "$SMPROGRAMS\Naali Player ${VERSION}"
  CreateShortCut "$SMPROGRAMS\Naali Player ${VERSION}\Naali Player ${VERSION}.lnk" "$INSTDIR\viewerp.exe"
  CreateShortCut "$SMPROGRAMS\Naali Player ${VERSION}\Readme.lnk" "$INSTDIR\readme.txt"
  CreateShortCut "$SMPROGRAMS\Naali Player ${VERSION}\Uninstall.lnk" "$INSTDIR\uninstaller.exe"
SectionEnd

Section "Uninstall"
  RMDir /r $INSTDIR
  
  #removing the APPDATA\roaming\realxtend doesn't seem to work properly on Vista. (maybe incorrect user information is given through add/remove programs)
  
  # LETS EVEN PROMPT USER IF HE WANT TO REMOVE ALL PERSONAL REX DATA (ether, old viewer data, assets, textures) BEFORE WE GO AHEAD AND DELETE THEM
  
  #SetShellVarContext all
  #RMDir /r "$APPDATA\Roaming\realXtend"
  #RMDir /r "$SMPROGRAMS\Naali ${VERSION}"

  #SetShellVarContext current
  #RMDir /r "$APPDATA\Roaming\realXtend"
  #RMDir /r "$SMPROGRAMS\Naali ${VERSION}"

  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\NaaliPlayer"
SectionEnd

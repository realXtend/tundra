!include "fileassoc.nsh"
!define VERSION "1.0.5"

Name "Tundra ${VERSION}"

Page directory
Page instfiles

InstallDir "$PROGRAMFILES\Tundra ${VERSION}"
VIProductVersion "${VERSION}"

OutFile "Tundra-${VERSION}.exe"

XPStyle on

RequestExecutionLevel admin

Section ""
  SetOutPath $INSTDIR
  File /r build\*.*

  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Tundra" \
                   "DisplayName" "Tundra ${VERSION}"

  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Tundra" \
                   "DisplayVersion" "${VERSION}"

  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Tundra" \
                   "UninstallString" "$INSTDIR\uninstaller.exe"

  # Register file extensions for .txml and .tbin.
  !insertmacro APP_ASSOCIATE "txml" "Tundra.Scenexmlfile" "Tundra XML Scene File" "$INSTDIR\data\ui\images\naali_icon.ico,0" "Open in Tundra Server" "$INSTDIR\server.exe --file $\"%1$\""
  !insertmacro APP_ASSOCIATE_ADDVERB "Tundra.Scenexmlfile" "openviewer" "Open Tundra Viewer in this directory" "$INSTDIR\viewer.exe --storage $\"%1$\""

  !insertmacro APP_ASSOCIATE "tbin" "Tundra.Scenebinfile" "Tundra Binary Scene File" "$INSTDIR\data\ui\images\naali_icon.ico,0" "Open in Tundra Server" "$INSTDIR\server.exe --file $\"%1$\""
  !insertmacro APP_ASSOCIATE_ADDVERB "Tundra.Scenebinfile" "openviewer" "Open Tundra Viewer in this directory" "$INSTDIR\viewer.exe --storage $\"%1$\""
  
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
  CreateDirectory "$SMPROGRAMS\Tundra ${VERSION}"
  CreateShortCut "$SMPROGRAMS\Tundra ${VERSION}\Tundra ${VERSION} Server.lnk" "$INSTDIR\server.exe"
  CreateShortCut "$SMPROGRAMS\Tundra ${VERSION}\Tundra ${VERSION} Viewer.lnk" "$INSTDIR\viewer.exe"
  CreateShortCut "$SMPROGRAMS\Tundra ${VERSION}\Readme.lnk" "$INSTDIR\readme.txt"
  CreateShortCut "$SMPROGRAMS\Tundra ${VERSION}\Uninstall.lnk" "$INSTDIR\uninstaller.exe"

  CreateDirectory "$SMPROGRAMS\Tundra ${VERSION}\Demos"
  CreateShortCut "$SMPROGRAMS\Tundra ${VERSION}\Demos\3DCanvas.lnk" "$INSTDIR\scenes\3DCanvas\TestScene.txml"
  CreateShortCut "$SMPROGRAMS\Tundra ${VERSION}\Demos\Avatar.lnk" "$INSTDIR\scenes\Avatar\avatar.txml"
  CreateShortCut "$SMPROGRAMS\Tundra ${VERSION}\Demos\ChatApplication.lnk" "$INSTDIR\scenes\ChatApplication\TestWorld.txml"
  CreateShortCut "$SMPROGRAMS\Tundra ${VERSION}\Demos\DayNight.lnk" "$INSTDIR\scenes\DayNight\TestWorld.txml"
  CreateShortCut "$SMPROGRAMS\Tundra ${VERSION}\Demos\ECSound.lnk" "$INSTDIR\scenes\ECSound\TestWorld.txml"
  CreateShortCut "$SMPROGRAMS\Tundra ${VERSION}\Demos\MumbleUI.lnk" "$INSTDIR\scenes\MumbleUI\test_mumble.txml"
  CreateShortCut "$SMPROGRAMS\Tundra ${VERSION}\Demos\Physics.lnk" "$INSTDIR\scenes\Physics\physics.txml"
  CreateShortCut "$SMPROGRAMS\Tundra ${VERSION}\Demos\RenderToTexture.lnk" "$INSTDIR\scenes\RenderToTexture\RttDemoWorld.txml"
  CreateShortCut "$SMPROGRAMS\Tundra ${VERSION}\Demos\Tooltip.lnk" "$INSTDIR\scenes\Tooltip\TestWorld.txml"
#  CreateShortCut "$SMPROGRAMS\Tundra ${VERSION}\Demos\WaterPlane.lnk" "$INSTDIR\scenes\WaterPlane\TestScene.txml"
SectionEnd

Section "Uninstall"
  RMDir /r $INSTDIR
  
  #removing the APPDATA\roaming\realxtend doesn't seem to work properly on Vista. (maybe incorrect user information is given through add/remove programs)
  
  # LETS EVEN PROMPT USER IF HE WANT TO REMOVE ALL PERSONAL REX DATA (ether, old viewer data, assets, textures) BEFORE WE GO AHEAD AND DELETE THEM
  
  #SetShellVarContext all
  #RMDir /r "$APPDATA\Roaming\realXtend"
  #RMDir /r "$SMPROGRAMS\Tundra ${VERSION}"

  #SetShellVarContext current
  #RMDir /r "$APPDATA\Roaming\realXtend"
  #RMDir /r "$SMPROGRAMS\Tundra ${VERSION}"

  # Unassociate file extension handlers 
  !insertmacro APP_UNASSOCIATE "txml" "Tundra.Scenexmlfile"
  !insertmacro APP_UNASSOCIATE "tbin" "Tundra.Scenebinfile"

  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Tundra"
SectionEnd

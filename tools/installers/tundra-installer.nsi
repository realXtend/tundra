!include "fileassoc.nsh"

# Note: You can define custom version from outside this script by using /DVERSION=YourCustomVersion
!ifndef VERSION
!define VERSION "2.0"
!endif

Name "Tundra ${VERSION}"

Page directory
Page instfiles

InstallDir "$PROGRAMFILES\Tundra ${VERSION}"
VIProductVersion "${VERSION}"

OutFile "realXtend-Tundra-${VERSION}.exe"

XPStyle on

RequestExecutionLevel admin

Section ""
  SetOutPath $INSTDIR

  # We are inside tools\installers
  File /r ..\..\build\*.*

  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Tundra" \
                   "DisplayName" "Tundra ${VERSION}"

  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Tundra" \
                   "DisplayVersion" "${VERSION}"

  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Tundra" \
                   "UninstallString" "$INSTDIR\uninstaller.exe"

  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Tundra" \
                   "QuietUninstallString" "$INSTDIR\uninstaller.exe /S"

  # Register file extensions for .txml and .tbin.
  !insertmacro APP_ASSOCIATE "txml" "Tundra.Scenexmlfile" "Tundra XML Scene File" "$INSTDIR\data\ui\images\icon\TundraLogo32px.ico,0" "Edit in Tundra" "$INSTDIR\tundra.exe --file $\"%1$\""
  !insertmacro APP_ASSOCIATE_ADDVERB "Tundra.Scenexmlfile" "hostserverheadless" "Host in Tundra Server (UDP 2345)" "$INSTDIR\tundra.exe --file $\"%1$\" --server 2345 --protocol udp --headless"
  !insertmacro APP_ASSOCIATE_ADDVERB "Tundra.Scenexmlfile" "hostwindowedserver" "Host in Windowed Tundra Server (UDP 2345)" "$INSTDIR\tundra.exe --file $\"%1$\" --server 2345 --protocol udp"
  !insertmacro APP_ASSOCIATE_ADDVERB "Tundra.Scenexmlfile" "openviewer" "Open Tundra Viewer in this Project Folder" "$INSTDIR\tundra.exe --config viewer.xml --storage $\"%1$\""

  !insertmacro APP_ASSOCIATE "tbin" "Tundra.Scenexmlfile" "Tundra Binary Scene File" "$INSTDIR\data\ui\images\icon\TundraLogo32px.ico,0" "Edit in Tundra" "$INSTDIR\tundra.exe --file $\"%1$\""
  !insertmacro APP_ASSOCIATE_ADDVERB "Tundra.Scenexmlfile" "hostserverheadless" "Host in Tundra Server (UDP 2345)" "$INSTDIR\tundra.exe --file $\"%1$\" --server 2345 --protocol udp --headless"
  !insertmacro APP_ASSOCIATE_ADDVERB "Tundra.Scenexmlfile" "hostwindowedserver" "Host in Windowed Tundra Server (UDP 2345)" "$INSTDIR\tundra.exe --file $\"%1$\" --server 2345 --protocol udp"
  !insertmacro APP_ASSOCIATE_ADDVERB "Tundra.Scenexmlfile" "openviewer" "Open Tundra Viewer in this Project Folder" "$INSTDIR\tundra.exe --config viewer.xml --storage $\"%1$\""

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
  CreateShortCut "$SMPROGRAMS\Tundra ${VERSION}\Tundra ${VERSION} Serve an Empty Scene.lnk" "$INSTDIR\tundra.exe" "--server 2345 --protocol udp"
  CreateShortCut "$SMPROGRAMS\Tundra ${VERSION}\Tundra ${VERSION} Start Offline in an Empty Scene.lnk" "$INSTDIR\tundra.exe" ""
  CreateShortCut "$SMPROGRAMS\Tundra ${VERSION}\Tundra ${VERSION} Viewer.lnk" "$INSTDIR\tundra.exe" "--config viewer.xml"
  CreateShortCut "$SMPROGRAMS\Tundra ${VERSION}\Uninstall.lnk" "$INSTDIR\uninstaller.exe"

  CreateDirectory "$SMPROGRAMS\Tundra ${VERSION}\Demos"
  CreateShortCut "$SMPROGRAMS\Tundra ${VERSION}\Demos\Avatar.lnk" "$INSTDIR\scenes\Avatar\avatar.txml"
  CreateShortCut "$SMPROGRAMS\Tundra ${VERSION}\Demos\ChatApplication.lnk" "$INSTDIR\scenes\ChatApplication\ChatApplication.txml"
  CreateShortCut "$SMPROGRAMS\Tundra ${VERSION}\Demos\Compositor.lnk" "$INSTDIR\scenes\Compositor\compositor.txml"
  CreateShortCut "$SMPROGRAMS\Tundra ${VERSION}\Demos\Compositor.lnk" "$INSTDIR\scenes\Compositor\compositor.txml"
  CreateShortCut "$SMPROGRAMS\Tundra ${VERSION}\Demos\DayNight.lnk" "$INSTDIR\scenes\DayNight\Scene.txml"
  CreateShortCut "$SMPROGRAMS\Tundra ${VERSION}\Demos\ScriptApp\ScriptApp.lnk" "$INSTDIR\scenes\ScriptApp\scriptapp.txml"
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

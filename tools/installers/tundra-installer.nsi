!include "fileassoc.nsh"

# Note: You can define custom version from outside this script by using /DVERSION=YourCustomVersion
!ifndef VERSION
!define VERSION "2.5"
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

  # Register URL handler for 'tundra://'.
  WriteRegStr HKCR "tundra" "" "URL:tundra Protocol"
  WriteRegStr HKCR "tundra" "URL Protocol" ""
  WriteRegStr HKCR "tundra\DefaultIcon" "" "$INSTDIR\data\ui\images\icon\TundraLogo32px.ico,0"
  WriteRegStr HKCR "tundra\shell\open\command" "" "$INSTDIR\tundra.exe --login $\"%1$\""

  ExecWait '"$INSTDIR\oalinst.exe" /S'
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
  CreateShortCut "$SMPROGRAMS\Tundra ${VERSION}\How to host a scene.lnk" "$INSTDIR\dox\hostserver.txt" ""
  CreateShortCut "$SMPROGRAMS\Tundra ${VERSION}\How to work offline.lnk" "$INSTDIR\dox\workoffline.txt" ""
  CreateShortCut "$SMPROGRAMS\Tundra ${VERSION}\How to connect to an existing server.lnk" "$INSTDIR\dox\howtoconnect.txt" ""
  CreateShortCut "$SMPROGRAMS\Tundra ${VERSION}\Tundra ${VERSION} UI-less Viewer.lnk" "$INSTDIR\tundra.exe" "--config viewer.xml"
  CreateShortCut "$SMPROGRAMS\Tundra ${VERSION}\Tundra ${VERSION} Browser.lnk" "$INSTDIR\tundra.exe" "--config viewer-browser.xml"
  CreateShortCut "$SMPROGRAMS\Tundra ${VERSION}\Uninstall.lnk" "$INSTDIR\uninstaller.exe"

  CreateDirectory "$SMPROGRAMS\Tundra ${VERSION}\Offline Demos"
  CreateShortCut "$SMPROGRAMS\Tundra ${VERSION}\Offline Demos\How to run offline demos.lnk" "$INSTDIR\dox\offlinedemos.txt" ""
  CreateShortCut "$SMPROGRAMS\Tundra ${VERSION}\Offline Demos\Animated UI.lnk" "$INSTDIR\tundra.exe" "--file $\"$INSTDIR\scenes\AnimatedUI\scene.txml$\""
  CreateShortCut "$SMPROGRAMS\Tundra ${VERSION}\Offline Demos\AssImpDemo.lnk" "$INSTDIR\tundra.exe" "--file $\"$INSTDIR\scenes\AssImpDemo\scene.txml$\""
  CreateShortCut "$SMPROGRAMS\Tundra ${VERSION}\Offline Demos\Compositor.lnk" "$INSTDIR\tundra.exe" "--file $\"$INSTDIR\scenes\Compositor\scene.txml$\""
  CreateShortCut "$SMPROGRAMS\Tundra ${VERSION}\Offline Demos\Day-Night.lnk" "$INSTDIR\tundra.exe" "--file $\"$INSTDIR\scenes\DayNight\scene.txml$\""
  CreateShortCut "$SMPROGRAMS\Tundra ${VERSION}\Offline Demos\Glow.lnk" "$INSTDIR\tundra.exe" "--file $\"$INSTDIR\scenes\Glow\scene.txml$\""
  CreateShortCut "$SMPROGRAMS\Tundra ${VERSION}\Offline Demos\Sound Component.lnk" "$INSTDIR\tundra.exe" "--file $\"$INSTDIR\scenes\ECSound\scene.txml$\""
  CreateShortCut "$SMPROGRAMS\Tundra ${VERSION}\Offline Demos\Physics.lnk" "$INSTDIR\tundra.exe" "--file $\"$INSTDIR\scenes\Physics\scene.txml$\""
  CreateShortCut "$SMPROGRAMS\Tundra ${VERSION}\Offline Demos\Render to Texture.lnk" "$INSTDIR\tundra.exe" "--file $\"$INSTDIR\scenes\RenderToTexture\scene.txml$\""
  CreateShortCut "$SMPROGRAMS\Tundra ${VERSION}\Offline Demos\Tooltip.lnk" "$INSTDIR\tundra.exe" "--file $\"$INSTDIR\scenes\Tooltip\scene.txml$\""

  CreateDirectory "$SMPROGRAMS\Tundra ${VERSION}\Server-Client Demos"
  CreateShortCut "$SMPROGRAMS\Tundra ${VERSION}\Server-Client Demos\How to run server-client demos.lnk" "$INSTDIR\dox\serverclientdemos.txt" ""
  CreateShortCut "$SMPROGRAMS\Tundra ${VERSION}\Server-Client Demos\Host Avatar Demo Locally.lnk" "$INSTDIR\tundra.exe" "--file $\"$INSTDIR\scenes\Avatar\scene.txml$\" --server --headless"
  CreateShortCut "$SMPROGRAMS\Tundra ${VERSION}\Server-Client Demos\Host ChatApplication Demo Locally.lnk" "$INSTDIR\tundra.exe" "--file $\"$INSTDIR\scenes\ChatApplication\scene.txml$\" --server --headless"
  CreateShortCut "$SMPROGRAMS\Tundra ${VERSION}\Server-Client Demos\Host EntityMoveTest Demo Locally.lnk" "$INSTDIR\tundra.exe" "--file $\"$INSTDIR\scenes\Tests\EntityMoveTest\scene.txml$\" --server --headless"
  CreateShortCut "$SMPROGRAMS\Tundra ${VERSION}\Server-Client Demos\Host ReplicationTest Demo Locally.lnk" "$INSTDIR\tundra.exe" "--file $\"$INSTDIR\scenes\Tests\ReplicationTest\scene.txml$\" --server --headless"
  CreateShortCut "$SMPROGRAMS\Tundra ${VERSION}\Server-Client Demos\Host SynchronizedPhysicsTest Demo Locally.lnk" "$INSTDIR\tundra.exe" "--file $\"$INSTDIR\scenes\Physics2\scene.txml$\" --server --headless"
  CreateShortCut "$SMPROGRAMS\Tundra ${VERSION}\Server-Client Demos\Connect to Localhost.lnk" "$INSTDIR\tundra.exe" "--connect localhost;2345;udp;avatar"

  CreateDirectory "$SMPROGRAMS\Tundra ${VERSION}\Visit Online"
  CreateShortCut "$SMPROGRAMS\Tundra ${VERSION}\Visit Online\LudoCraft Circus.lnk" "http://tundra.ludocraft.com/circus/" ""
  CreateShortCut "$SMPROGRAMS\Tundra ${VERSION}\Visit Online\RealXtend Login Portal.lnk" "http://login.realxtend.org/" ""

  CreateDirectory "$SMPROGRAMS\Tundra ${VERSION}\Developers"
  CreateShortCut "$SMPROGRAMS\Tundra ${VERSION}\Developers\Binary Downloads.lnk" "http://code.google.com/p/realxtend-naali/downloads/list" ""
  CreateShortCut "$SMPROGRAMS\Tundra ${VERSION}\Developers\Github Source Repository.lnk" "https://github.com/realXtend/naali" ""
  CreateShortCut "$SMPROGRAMS\Tundra ${VERSION}\Developers\Online Documentation.lnk" "http://www.realxtend.org/doxygen/" ""
  CreateShortCut "$SMPROGRAMS\Tundra ${VERSION}\Developers\License.lnk" "$INSTDIR\LICENSE.txt" ""
  CreateShortCut "$SMPROGRAMS\Tundra ${VERSION}\Developers\What's New.lnk" "$INSTDIR\WhatsNew.txt" ""
  CreateShortCut "$SMPROGRAMS\Tundra ${VERSION}\Developers\Report a Bug.lnk" "https://github.com/realXtend/naali/issues" ""
SectionEnd

Section "Uninstall"
  RMDir /r $INSTDIR

  SetShellVarContext all
  RMDir /r "$SMPROGRAMS\Tundra ${VERSION}"

  # Remove the registered URL handler.
  DeleteRegKey HKCR "tundra"

  # Unassociate file extension handlers
  !insertmacro APP_UNASSOCIATE "txml" "Tundra.Scenexmlfile"
  !insertmacro APP_UNASSOCIATE "tbin" "Tundra.Scenebinfile"

  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Tundra"
SectionEnd

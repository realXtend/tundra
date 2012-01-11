/** For conditions of distribution and use, see copyright notice in LICENSE

    Instructions.js - Instruction overlay for EntityMoveTest demo scene. */

var showUi = true;
if (server.IsRunning() && framework.IsHeadless())
    showUi = false;

if (showUi)
{
    engine.ImportExtension("qt.core");
    engine.ImportExtension("qt.gui");

    var label = new QLabel();
    label.indent = 10;
    label.text = "This application stress-tests the object movement replication by constantly moving multiple entities.\n" +
        "Edit EntityMoveTest.js to alter the number of boxes.\n" + 
        "Remember to start server with \"--server\" command line parameter";
    label.resize(750,350);
    label.setStyleSheet("QLabel {background-color: transparent; font-size: 16px; }");

    var proxy = new UiProxyWidget(label);
    ui.AddProxyWidgetToScene(proxy);
    proxy.x = 50
    proxy.y = 50;
    proxy.windowFlags = 0;
    proxy.visible = true;
    proxy.focusPolicy = Qt.NoFocus;
}

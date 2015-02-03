/** For conditions of distribution and use, see copyright notice in LICENSE

    Instructions.js - Instruction overlay for InfiniteWorld demo scene. */

var showUi = true;
if (server.IsRunning() && framework.IsHeadless())
    showUi = false;

if (showUi)
{
    engine.ImportExtension("qt.core");
    engine.ImportExtension("qt.gui");

    var label = new QLabel();
    label.indent = 10;
    label.text = "InfiniteWorld ia a testbed/demo scene for interest management.\nSee InfiniteWorld.js for configurable parameters."
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

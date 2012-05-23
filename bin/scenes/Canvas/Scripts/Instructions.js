/** For conditions of distribution and use, see copyright notice in LICENSE

    Instructions.js - Instruction overlay for Canvas demo scene. */

if (!framework.IsHeadless())
{
    engine.ImportExtension("qt.core");
    engine.ImportExtension("qt.gui");

    var label = new QLabel();
    label.indent = 10;
    label.text = "This application This scene demonstrates the GraphicsViewCanvas component.\n" +
        "GraphicsViewCanvas can be used to embed arbitrary Qt UI elements into 3D models with working mouse input." ;
    label.resize(650, 100);
    label.setStyleSheet("QLabel {background-color: transparent; font-size: 16px; }");

    var proxy = new UiProxyWidget(label);
    ui.AddProxyWidgetToScene(proxy);
    proxy.x = 50
    proxy.y = 50;
    proxy.windowFlags = 0;
    proxy.visible = true;
    proxy.focusPolicy = Qt.NoFocus;
}

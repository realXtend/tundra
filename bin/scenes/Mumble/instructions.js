engine.ImportExtension("qt.core");
engine.ImportExtension("qt.gui");

var label = new QLabel();
label.indent = 10;
label.text = "This scene demonstrates adding VOIP support to your scene.";
label.resize(660,150);
label.setStyleSheet("QLabel {background-color: transparent; font-size: 16px; }");

var proxy = new UiProxyWidget(label);
ui.AddProxyWidgetToScene(proxy);
proxy.x = 90;
proxy.y = 90;
proxy.windowFlags = 0;
proxy.visible = true;

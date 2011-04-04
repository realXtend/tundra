engine.ImportExtension("qt.core");
engine.ImportExtension("qt.gui");

var label = new QLabel();
label.indent = 10;
label.text = "This scene uses a script and the HoveringText component to implement\nmouse-hover tooltips on objects.";
label.resize(660,100);
label.setStyleSheet("QLabel {background-color: transparent; font-size: 16px; }");

var proxy = new UiProxyWidget(label);
ui.AddProxyWidgetToScene(proxy);
proxy.x = 100;
proxy.y = 30;
proxy.windowFlags = 0;
proxy.visible = true;

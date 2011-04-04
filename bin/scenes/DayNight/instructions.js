engine.ImportExtension("qt.core");
engine.ImportExtension("qt.gui");

var label = new QLabel();
label.indent = 10;
label.text = "This scene demonstrates the EnvironmentLight component. This\ncomponentc ontrols the overall environmental lighting of the scene.\n\nA simple script is used to animate the day-night cycle of the scene.";
label.resize(660,150);
label.setStyleSheet("QLabel {background-color: transparent; color: blue; font-size: 20px; }");

var proxy = new UiProxyWidget(label);
ui.AddProxyWidgetToScene(proxy);
proxy.x = 100;
proxy.y = 30;
proxy.windowFlags = 0;
proxy.visible = true;

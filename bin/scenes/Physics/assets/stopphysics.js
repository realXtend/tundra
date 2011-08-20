engine.ImportExtension("qt.core");
engine.ImportExtension("qt.gui");

physics.SetRunPhysics(false);

var label = new QLabel();
label.indent = 10;
label.text = "To start the physics simulation, open the console by pressing F1, and type in \"startphysics\".";
label.resize(660,30);
label.setStyleSheet("QLabel {background-color: transparent; font-size: 16px; }");

var proxy = new UiProxyWidget(label);
ui.AddProxyWidgetToScene(proxy);
proxy.x = 100;
proxy.y = 30;
proxy.windowFlags = 0;
proxy.visible = true;

engine.ImportExtension("qt.core");
engine.ImportExtension("qt.gui");

var label = new QLabel();
label.indent = 10;
label.text = "This scene implements a text chat application. In this world, clients can chat\nwith each other using a provided chat widget. This logic is implemented in the scene\nentity ChatApplication, which contains the script ChatApplication.js. You can embed\nthis chat application to your scene by copying this object to your world.";
label.resize(660,500);
label.setStyleSheet("QLabel {background-color: transparent; font-size: 16px; }");

var proxy = new UiProxyWidget(label);
uiservice.AddProxyWidgetToScene(proxy);
proxy.x = 100;
proxy.y = 30;
proxy.windowFlags = 0;
proxy.visible = true;
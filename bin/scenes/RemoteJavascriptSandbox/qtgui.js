//(at least) selected parts of the qt gui widget set
//are exposed to untrusted code too. ok?

engine.ImportExtension("qt.core");
engine.ImportExtension("qt.gui");

var label = new QLabel();
label.indent = 10;
label.text = "This scene demonstrates how an untrusted js file\nfrom the net can use qt gui widgets normally.";
label.resize(660,150);
label.setStyleSheet("QLabel {background-color: transparent; color: blue; font-size: 20px; }");

var proxy = new UiProxyWidget(label);
uiservice.AddProxyWidgetToScene(proxy);
proxy.x = 100;
proxy.y = 30;
proxy.windowFlags = 0;
proxy.visible = true;
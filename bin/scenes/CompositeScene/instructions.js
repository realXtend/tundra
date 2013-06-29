engine.ImportExtension("qt.core");
engine.ImportExtension("qt.gui");

var label = new QLabel();
label.indent = 10;
label.text = "This is a simple implementation of compositing a scene from parts.\n"
             + "It is implemented with a Javascript-written scene part manager.\n"
             + "It works by finding all entities with a DynamicComponent called Scenepart,\n"
             + "loading the TXML referred in the assetref attribute there, using parenting to place it.\n\n"
             + "See the associated scenepartmanager.js and the master scene and part txmls for details.";
label.resize(680,120);
label.setStyleSheet("QLabel {background-color: transparent; font-size: 16px; }");

var proxy = new UiProxyWidget(label);
ui.AddProxyWidgetToScene(proxy);
proxy.x = 100;
proxy.y = 40;
proxy.windowFlags = 0;
proxy.visible = true;

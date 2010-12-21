engine.ImportExtension("qt.core");
engine.ImportExtension("qt.gui");

var label = new QLabel();
label.indent = 10;
label.text = "This scene demonstrates the use of the 3DCanvas and 3DCanvasSource components.\nThese components together can be used to show a webpage as a texture on a 3D object.\nAdjust the attributes of the 3DCanvasSource to change the webpage that is displayed.";
label.resize(660,70);
label.setStyleSheet("QLabel {background-color: transparent; font-size: 16px; }");

var proxy = new UiProxyWidget(label);
uiservice.AddProxyWidgetToScene(proxy);
proxy.x = 100;
proxy.y = 30;
proxy.windowFlags = 0;
proxy.visible = true;
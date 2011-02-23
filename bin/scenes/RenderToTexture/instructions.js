engine.ImportExtension("qt.core");
engine.ImportExtension("qt.gui");

var label = new QLabel();
label.indent = 10;
label.text = "This scene demonstrates render-to-texture support.\n"
             + "You can use it to create external renders of any scene camera.\n"
             + "Press the square buttons next to the big display to switch the source camera.\n\n"
             + "See the associated rttdisplay.js for details.";
label.resize(660,100);
label.setStyleSheet("QLabel {background-color: transparent; font-size: 16px; }");

var proxy = new UiProxyWidget(label);
uiservice.AddProxyWidgetToScene(proxy);
proxy.x = 100;
proxy.y = 30;
proxy.windowFlags = 0;
proxy.visible = true;
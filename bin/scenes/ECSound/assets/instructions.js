engine.ImportExtension("qt.core");
engine.ImportExtension("qt.gui");

var label = new QLabel();
label.indent = 10;
label.text = "This scene demonstrates how to use the Sound component. This component\nallows you to add spatial sound sources to the scene.\n\nThe freelook camera contains a SoundListener component, which acts as the \"ears\"\nfor receiving spatial audio.";
label.resize(660,150);
label.setStyleSheet("QLabel {background-color: transparent; font-size: 16px; }");

var proxy = new UiProxyWidget(label);
ui.AddProxyWidgetToScene(proxy);
proxy.x = 100;
proxy.y = 30;
proxy.windowFlags = 0;
proxy.visible = true;

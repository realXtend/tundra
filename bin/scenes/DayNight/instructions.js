engine.ImportExtension("qt.core");
engine.ImportExtension("qt.gui");

var label = new QLabel();
label.indent = 10;
label.text = "This scene demonstrates the SkyX and Hydrax plugin integrations\nto Tundra.\n\nThe SkyX plugin implements an outdoor environment with a\ncontrollable day-night cycle. The Hydrax plugin implements a\nprocedurally generated water effect.";
label.resize(660,150);
label.setStyleSheet("QLabel {background-color: transparent; color: blue; font-size: 20px; }");

var proxy = new UiProxyWidget(label);
ui.AddProxyWidgetToScene(proxy);
proxy.x = 100;
proxy.y = 60;
proxy.windowFlags = 0;
proxy.visible = true;

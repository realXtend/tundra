engine.ImportExtension("qt.core");
engine.ImportExtension("qt.gui");

var label = new QLabel();
label.wordWrap = true;
label.indent = 10;
label.text = "This scene demonstrates the SkyX and Hydrax plugin integrations to Tundra.\n" +
    "The SkyX plugin implements an outdoor environment with a controllable day-night cycle. " +
    "The Hydrax plugin implements a procedurally generated water effect.\n\n" +
    "The scene also demonstrates how to implement an object floating on the water.";
label.resize(700,300);
label.setStyleSheet("QLabel {background-color: transparent; color: white; font-size: 20px; }");

var proxy = new UiProxyWidget(label);
ui.AddProxyWidgetToScene(proxy);
proxy.x = 100;
proxy.y = 60;
proxy.windowFlags = 0;
proxy.visible = true;

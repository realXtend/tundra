print("Loading time counter example script.");

// Load ui file and attach it to proxywidget.
var widget = ui.LoadFromFile(".\\jsmodules\\apitest\\time_counter\\time_counter.ui", false);
var lcdWidget = widget.findChild("lcdNumber");
lcdWidget.value = 0.0;
var proxy = new UiProxyWidget(widget);
ui.AddWidgetToScene(proxy);
proxy.x = 600;
proxy.y = 50;
proxy.visible = true;
proxy.windowFlags = 0;

function delayedMessage(time)
{
    lcdWidget.value = lcdWidget.value + 1;
    frame.DelayedExecute(1.0).Triggered.connect(delayedMessage);
}
frame.DelayedExecute(1.0).Triggered.connect(delayedMessage);

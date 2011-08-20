print("Loading ui example script.");

// Widget that will get destoyed when the script instance is termited.
var widget = ui.LoadFromFile("./jsmodules/apitest/time_counter/time_counter.ui", false);
var proxy = new UiProxyWidget(widget);
ui.AddProxyWidgetToScene(proxy);
proxy.x = 600;
proxy.y = 50;
proxy.visible = true;
proxy.windowFlags = 0; //Qt::Widget

// Same but the widget wont get destoyed when the script instance is termited.
var widget2 = ui.LoadFromFile("./jsmodules/apitest/time_counter/time_counter.ui", false);
var proxy2 = ui.AddWidgetToScene(widget2);
proxy2.x = 600;
proxy2.y = 50;
proxy2.visible = true;
proxy2.windowFlags = 3; //Qt::Dialog

function exit(time)
{
    print(time + " seconds has passed, exit.");
    framework.Exit()
}

var delayed2 = frame.DelayedExecute(3.0);
delayed2.Triggered.connect(exit);

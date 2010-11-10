print("Loading ui example script.");

// Widget that will get destoyed when the script instance is termited.
var widget = uiservice.LoadFromFile(".\\jsmodules\\apitest\\time_counter\\time_counter.ui", false);
var proxy = new UiProxyWidget(widget);
uiservice.AddWidgetToScene(proxy);
proxy.x = 600;
proxy.y = 50;
proxy.visible = true;
proxy.windowFlags = 0; //Qt::Widget

// Same but the widget wont get destoyed when the script instance is termited.
var widget2 = uiservice.LoadFromFile(".\\jsmodules\\apitest\\time_counter\\time_counter.ui", false);
var proxy2 = uiservice.AddWidgetToScene(widget2);
proxy2.x = 600;
proxy2.y = 50;
proxy2.visible = true;
proxy2.windowFlags = 3; //Qt::Dialog
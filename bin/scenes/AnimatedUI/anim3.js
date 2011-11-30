// !ref: anim3.ui

engine.ImportExtension("qt.core");
engine.ImportExtension("qt.gui");

var menu = asset.GetAsset("anim3.ui").Instantiate(true, 0);
menu.move(0,0);
menu.show();

var proxy = menu.graphicsProxyWidget();
proxy.windowFlags = 0;
proxy.visible = true;
proxy.focusPolicy = Qt.NoFocus;

frame.Updated.connect(OnFrameUpdate);
function OnFrameUpdate()
{
   menu.setWindowOpacity(Math.sin(frame.WallClockTime()*2)*0.5 + 0.5);
   menu.lower();
}

/* BUG:

var wnd = new QWidget();
var b1 = new QPushButton(wnd);
wnd.width = 100; // This does NOT resize the widget! But if you comment out the following line, it does.
ui.AddWidgetToScene(wnd);
wnd.show();
*/

/* As a workaround to above, you will have to do this:
var wnd = new QWidget();
var b1 = new QPushButton(wnd);
var proxy = ui.AddWidgetToScene(wnd);
wnd.show();
proxy.width = 100;
*/

function OnScriptDestroyed()
{
  menu.hide();
  menu.deleteLater();
}

// !ref: TestUI.ui

engine.ImportExtension("qt.core");
engine.ImportExtension("qt.gui");

var menu = asset.GetAsset("TestUI.ui").Instantiate(true, 0);
menu.move(50, 100);
menu.show();

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

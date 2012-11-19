print("Loading ui example script.");
engine.ImportExtension("qt.core")
engine.ImportExtension("qt.gui")
engine.ImportExtension("qt.uitools");

function MyWidget(parent)
{ 
    QWidget.call(this, parent);
}

// When resizeEvent is handled, increase lcd number by one.
MyWidget.prototype.resizeEvent = function(event) 
{
    var lcd = findChild(main, "lcdNumber");
    if (lcd)
        lcd.intValue = lcd.intValue + 1;
}

MyWidget.prototype.keyPressEvent = function(event)
{
    var lcd = findChild(main, "lcdNumber");
    if (!lcd)
        return;

    lcd.display(event.key());
}

// Widget that will get destroyed after the script instance is termited.
var main = new MyWidget("Testing");
var loader = new QUiLoader();
var file = new QFile("./jsmodules/Tests/Api/Ui/TimeCounter.ui");
file.open(QIODevice.ReadOnly);
var widget = loader.load(file, main);
file.close();
ui.AddWidgetToScene(main);
main.show();

// Same but the widget wont get destoyed after the script instance is termited.
/*var loader = new QUiLoader();
var file = new QFile(".\\jsmodules\\apitest\\time_counter\\time_counter.ui");
file.open(QIODevice.ReadOnly);
var widget = loader.load(file);
file.close();
uiservice.AddWidgetToScene(widget);
widget.show();*/

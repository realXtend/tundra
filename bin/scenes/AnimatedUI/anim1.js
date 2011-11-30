engine.ImportExtension("qt.core");
engine.ImportExtension("qt.gui");

var label = new QLabel();
label.objectName = "InfoLabel";
label.setStyleSheet("QLabel#InfoLabel { padding: 10px; background-color: rgba(230,230,230,10); border: 1px solid black; font-size: 16px; }");
label.text = "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod\n\
tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim\n\
veniam, quis nostrudexercitation ullamco laboris nisi ut aliquip ex ea\n\
commodo consequat. Duis aute irure dolor in reprehenderit in voluptate\n\
velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint\n\
occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit\n\
anim id est laborum.";

var proxy = ui.AddWidgetToScene(label);

proxy.windowFlags = 0;
proxy.visible = true;
proxy.focusPolicy = Qt.NoFocus;
proxy.x = 400;
proxy.y = 300;

frame.Updated.connect(OnFrameUpdate);
function OnFrameUpdate()
{
    var alpha = (Math.sin(frame.WallClockTime()*5)*0.5 + 0.5) * 255;
//    label.setStyleSheet("QLabel#InfoLabel { padding: 10px; background-color: rgba(230,230,230," + alpha + "); border: 1px solid black; font-size: 16px; }");
    label.resize(600 + Math.sin(frame.WallClockTime()) * 30, 300 + Math.cos(frame.WallClockTime()) * 100);
}

function OnScriptDestroyed()
{
  label.deleteLater();
}

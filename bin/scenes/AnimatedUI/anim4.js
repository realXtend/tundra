engine.ImportExtension("qt.core");
engine.ImportExtension("qt.gui");

var items = {}

for(i = 0; i < 10; ++i)
{
  var label = new QLabel();
  label.objectName = "InfoLabel";
  label.setStyleSheet("QLabel#InfoLabel { padding: 10px; background-color: rgba(230,230,230,175); border: 1px solid black; font-size: 16px; }");
  label.text = "Moving text!";

  var proxy = ui.AddWidgetToScene(label);

  // Check if the browser ui is present (or anything else on top left corner)
  proxy.x = 20 + i*150;
  proxy.y = ui.MainWindow().height/2;
  proxy.windowFlags = 0;
  proxy.visible = true;
  proxy.focusPolicy = Qt.NoFocus;
  items[i] = proxy;
}

frame.Updated.connect(OnFrameUpdate);
function OnFrameUpdate()
{
   for(i = 0; i < 10; ++i)
      items[i].y = ui.MainWindow().height/2 + Math.sin(frame.WallClockTime() + i * 3.1415/10) * ui.MainWindow().height/4;
}

engine.ImportExtension("qt.core");
engine.ImportExtension("qt.gui");

var label = null;
var proxy = null;

function OnScriptDestroyed()
{
    if (label != null)
    {
        label.visible = false;
        ui.RemoveWidgetFromScene(label);
    }
}

if (!framework.IsHeadless())
{
    label = new QLabel();
    label.styleSheet = "QLabel { background-color: rgba(230,230,230,170); font-size: 16px; padding: 10px; }";
    label.text = "<b>This scene demonstrates adding per connection scene state prioritization to you scene as server side logic</b><br>" +
                 "See more details from SyncStatePriority.js where you have logic explanation, run instructions and how to change the behaviour<br>" +
                 "If you dont have the avatar application running, please read the run instructions from SyncStatePriority.js no how to add it.<br><br>" +
                 "If you are not familiar with avatar controls run the avatar example first, see scenes/Avatar for instructions.";

    proxy = ui.AddWidgetToScene(label);
    proxy.pos = new QPointF(10, 10);
    proxy.windowFlags = 0;
    proxy.visible = true;
}
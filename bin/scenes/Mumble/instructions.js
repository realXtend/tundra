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
    label.text = "<b>This scene and its script MumbleApplication.js demonstrates adding VOIP support to your scene.</b><br><br>" +
                 "Tundras MumblePlugin provides a Mumble client that can connect to Murmur servers.<br>" +
                 "You can get this server and host it yourself: http://www.mumble.com/<br>" +
                 "Remember to check 'Install Murmur component' in the installer, the server is not installed by default.<br>" + 
                 "See murmur.ini for configuring and murmur executable to run the server.<br><br>" +
                 "Once your server is up, use the client UI in Tundra to connect to your server.<br>" +
                 "Read more on what you can do with MumblePlugin in the MumbleApplication.js and the Tundra documentation.";

    proxy = ui.AddWidgetToScene(label);
    proxy.pos = new QPointF(10, 10);
    proxy.windowFlags = 0;
    proxy.visible = true;
}
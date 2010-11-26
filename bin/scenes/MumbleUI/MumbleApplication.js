engine.ImportExtension("qt.core");
engine.ImportExtension("qt.gui");
engine.ImportExtension("qt.uitools");
print("Loading MumbleApplication");

var widget = uiservice.LoadFromFile(".\\scenes\\MumbleUI\\SettingsWidget.ui", false);	
var audioReceiving = findChild(widget, "audioReceiving");
var audioSending = findChild(widget, "audioSending");
var channelsList = findChild(widget, "channelsList");
var inWorldVoiceSession = communications_service.InWorldVoiceSession();
var chlist = inWorldVoiceSession.GetChannels();
uiservice.AddWidgetToScene(widget);

widget.x = 10;
widget.y = 10;
widget.visible = true;
widget.windowFlags = 0;

channelsList.addItems(chlist);

audioReceiving.stateChanged.connect(setAudioReceiving);
audioSending.stateChanged.connect(setAudioSending);
channelsList['currentIndexChanged(QString)'].connect(connectToChannel);

function setAudioReceiving(state)
{
    if (state)
    {
        inWorldVoiceSession.EnableAudioReceiving();
    } else
    {
        inWorldVoiceSession.DisableAudioReceiving();
    }
}

function setAudioSending(state)
{
    if (state)
    {
        inWorldVoiceSession.EnableAudioSending();
    } else 
    {
        inWorldVoiceSession.DisableAudioSending();
    }
}

function connectToChannel(channel)
{
    if (channel)
    {
        inWorldVoiceSession.SetActiveChannel(channel);
    }
}

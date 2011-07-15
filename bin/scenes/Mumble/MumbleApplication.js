// !ref: local://controls.ui

// Simple example script for using MumbleVoipModule as VOIP communications provider.

// Server information
var m_server = "chiru.cie.fi";
var m_port = "64738";
var m_version = "1.2.2";
var m_password = "";
var m_channels = new Array();

// Global variables
var sendEnabled = false;
var receiveEnabled = false;
var surroundEnabled = false;
var position = new Vector3df();

//  Channels we are aware of
//  Channel ID          Channel name
    m_channels[0] =     "Root";
    m_channels[1] =     "Root\\Subroot";

if (!server.IsRunning())
{
    engine.ImportExtension("qt.core");
    engine.ImportExtension("qt.gui");
    engine.ImportExtension("qt.uitools");
    
    var inWorldVoiceSession = communications_service.InWorldVoiceSession();
    var username = "User" + client.GetConnectionID();
    
    // Load UI
    var controlUI = ui.LoadFromFile("local://controls.ui", false);
    controlUI.windowTitle = "Settings";
    ui.AddWidgetToScene(controlUI);
    controlUI.visible = true;
    
    // Find UI controls
    var sendEnabledButton = findChild(controlUI, "enableSendButton");
    var receiveEnabledButton = findChild(controlUI, "enableReceiveButton");
    var surroundEnabledButton = findChild(controlUI, "enableSurroundButton");
    var posXEdit = findChild(controlUI, "xLineEdit");
    var posYEdit = findChild(controlUI, "yLineEdit");
    var posZEdit = findChild(controlUI, "zLineEdit");
    
    if (inWorldVoiceSession)
    {
        // Register channels to inworldvoicesession
        for(var i = 0; i < m_channels.length; i++)
        {
            inWorldVoiceSession.AddChannel(m_channels[i], username, m_server, m_port, m_password, m_version, i);
        }
        inWorldVoiceSession.SetActiveChannel(m_channels[0]); // Root
        inWorldVoiceSession.EnablePositionalAudio(false);
        
        // Connect UI controls
        sendEnabledButton.clicked.connect(setSendEnabled);
        receiveEnabledButton.clicked.connect(setReceiveEnabled);        
        surroundEnabledButton.clicked.connect(setSurroundEnabled);
        posXEdit.textChanged.connect(setPositionX);
        posXEdit.returnPressed.connect(updatePosition);
        posYEdit.textChanged.connect(setPositionY);
        posYEdit.returnPressed.connect(updatePosition);
        posZEdit.textChanged.connect(setPositionZ);
        posZEdit.returnPressed.connect(updatePosition);
    }

    function setSendEnabled()
    {
        sendEnabled = !sendEnabled;
        if(sendEnabled)
            inWorldVoiceSession.EnableAudioSending();
        else
            inWorldVoiceSession.DisableAudioSending();
    }
    
    function setReceiveEnabled()
    {
        receiveEnabled = !receiveEnabled;
        if(receiveEnabled)
            inWorldVoiceSession.EnableAudioReceiving();
        else
            inWorldVoiceSession.DisableAudioReceiving();
    }
    
    function setSurroundEnabled()
    {
        surroundEnabled = !surroundEnabled;
        inWorldVoiceSession.EnablePositionalAudio(surroundEnabled);
    }
    
    function setPositionX(value)
    {
        position.x = parseFloat(value);
    }
    
    function setPositionY(value)
    {
        position.y = parseFloat(value);
    }
    
    function setPositionZ(value)
    {
        position.z = parseFloat(value);
    }
    
    function updatePosition()
    {
        if(!isNaN(position.x) && !isNaN(position.y) && !isNaN(position.z))
            inWorldVoiceSession.SetPosition(position);
    }
}

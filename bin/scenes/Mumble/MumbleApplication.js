// !ref: local://controls.ui
// !ref: local://participants.ui

// Simple example script for using MumbleVoipModule as VOIP communications provider.

// You can edit your inworld voice position by using the 3D Sound controls, in practice
// the position update could be done in example AvatarApplications update loop, so that
// it reflects the actual position of the user.

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

//  Channels we wan't to use
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
    
    // Load controls UI
    var controlUI = ui.LoadFromFile("local://controls.ui", false);
    controlUI.windowTitle = "VoIP Settings";
    ui.AddWidgetToScene(controlUI);
    controlUI.visible = true;
    
    // Load participants UI
    var participantsUI = ui.LoadFromFile("local://participants.ui", false);
    participantsUI.windowTitle = "Participants";
    ui.AddWidgetToScene(participantsUI);
    participantsUI.visible = true;
    
    // Find UI controls
    var sendEnabledButton = findChild(controlUI, "enableSendButton");
    var receiveEnabledButton = findChild(controlUI, "enableReceiveButton");
    var surroundEnabledButton = findChild(controlUI, "enableSurroundButton");
    var posXEdit = findChild(controlUI, "xLineEdit");
    var posYEdit = findChild(controlUI, "yLineEdit");
    var posZEdit = findChild(controlUI, "zLineEdit");
    var channelWidget = findChild(controlUI, "channelComboBox");
    var participantWidget = findChild(participantsUI, "plTableWidget");
    
    if (inWorldVoiceSession)
    {
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
        channelWidget['currentIndexChanged(QString)'].connect(connectToChannel);
        
        inWorldVoiceSession.ChannelListChanged.connect(channelListChanged);
        inWorldVoiceSession.ActiceChannelChanged.connect(activeChannelChanged);
        inWorldVoiceSession.ParticipantJoined.connect(updateParticipantList);
        inWorldVoiceSession.ParticipantLeft.connect(updateParticipantList);
        
        // Register channels to inworldvoicesession
        for(var i = 0; i < m_channels.length; i++)
        {
            inWorldVoiceSession.AddChannel(m_channels[i], username, m_server, m_port, m_password, m_version, i);
        }
        inWorldVoiceSession.SetActiveChannel(m_channels[0]); // Root
        inWorldVoiceSession.EnablePositionalAudio(false);
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
    
    function channelListChanged(list)
    {
        if(list)
        {
            channelWidget.clear();
            channelWidget.addItems(list);
        }
    }
    
    function activeChannelChanged(channel)
    {
        if(channel)
            channelWidget.setCurrentIndex(channelWidget.findText(channel));
    }
    
    function connectToChannel(channel)
    {
        if (channel)
            inWorldVoiceSession.SetActiveChannel(channel);
    }
    
    function updateParticipantList()
    {
        if (inWorldVoiceSession)
        {
            clearParticipants();

            var participants = inWorldVoiceSession.GetParticipantsNames();

            var pArray = new Array(); 
            pArray = participants.toString().split(",");

            var i;
            var rowCount = participantWidget.rowCount;

            var muteButton = new Array();
            var buttonGroup = new QButtonGroup;

            for (i = 0; i < pArray.length; i++)
            {
                var p = new QTableWidgetItem(pArray[i]);
                participantWidget.insertRow(rowCount + i);
                participantWidget.setItem(rowCount + i, 0, p);
                muteButton[i] = new QPushButton("Mute");

                participantWidget.setCellWidget(rowCount + i, 1, muteButton[i]);
                buttonGroup.addButton(muteButton[i], rowCount+i);
            }
            buttonGroup['buttonClicked(int)'].connect(muteParticipant);
        }
    }
    
    function muteParticipant(row) 
    {
        if (inWorldVoiceSession)
        {
            var participant = participantWidget.item(row, 0).text();
            var muteButton = participantWidget.cellWidget(row, 1);
            if (muteButton.text == "Mute")
            {
                inWorldVoiceSession.MuteParticipantByName(participant.toString(), true);
                muteButton.text = "Unmute";
            } else
            {
                inWorldVoiceSession.MuteParticipantByName(participant.toString(), false);
                muteButton.text = "Mute";
            }
        }
    }
    
    function clearParticipants()
    {
        for (var i = participantWidget.rowCount-1; i >= 0; --i)
        {
            participantWidget.removeRow(i);
        }
    }
}

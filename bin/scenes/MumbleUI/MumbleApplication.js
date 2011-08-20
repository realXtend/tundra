// !ref: local://SettingsWidget.ui
// !ref: local://ParticipantsList.ui
// !ref: local://TransmissionMode.ui

if (!server.IsRunning())
{
    engine.ImportExtension("qt.core");
    engine.ImportExtension("qt.gui");
    engine.ImportExtension("qt.uitools");

    // Load ParticipantsList widget and hide
    var pw = ui.LoadFromFile("local://ParticipantsList.ui", false);
    var plTableWidget = findChild(pw, "plTableWidget");
    plTableWidget.setColumnWidth(0, 198);
    plTableWidget.setColumnWidth(1, 51);
    ui.AddWidgetToScene(pw);
    pw.hide();

    // Load TransmissionMode widget and hide
    var tm = ui.LoadFromFile("local://TransmissionMode.ui", false);
    var rbOff = findChild(tm, "rbOff");
    var rbCt = findChild(tm, "rbCt");
    ui.AddWidgetToScene(tm);
    tm.hide();

    // Load SettingWidget
    var widget = ui.LoadFromFile("local://SettingsWidget.ui", false);	
    var channelsList = findChild(widget, "channelsList");
    var transmissionType = findChild(widget, "transmissionType");
    var participantsList = findChild(widget, "participantsList");
    var numberOfParticipants = findChild(widget, "numberOfParticipants");
    var channelMsg = findChild(widget, "channelMsg");
    widget.resize(216, 50);
    widget.windowTitle = "Voice comm";

    var inWorldVoiceSession = communications_service.InWorldVoiceSession();

    if (inWorldVoiceSession)
    {
        var chlist = inWorldVoiceSession.GetChannels();
        ui.AddWidgetToScene(widget);

        widget.x = 10;
        widget.y = 10;
        widget.visible = true;
        widget.windowFlags = 0;

        channelsList.addItems(chlist);

        channelsList['currentIndexChanged(QString)'].connect(connectToChannel);
        participantsList.clicked.connect(getParticipants);
        transmissionType.clicked.connect(showTransmissionWidget);
        rbOff.clicked.connect(changeTransmissionType);
        rbCt.clicked.connect(changeTransmissionType);
    }

    function connectToChannel(channel)
    {
        if (channel)
        {
            inWorldVoiceSession.SetActiveChannel(channel);
            widget.resize(310, 50);

            participantsList.styleSheet = "border-image: url(./data/assets/user_green.png) 3 3 3 3; border-top: 3px transparent;border-bottom: 3px transparent;border-right: 3px transparent;border-left: 3px transparent;";
        }
    }

    function changeTransmissionType()
    {
        if (rbOff.checked)
        {
            inWorldVoiceSession.DisableAudioReceiving();
            inWorldVoiceSession.DisableAudioSending();
            transmissionType.styleSheet = "border-image: url(./data/assets/status_offline.png) 3 3 3 3; border-top: 3px transparent; border-bottom: 3px transparent; border-right: 3px transparent; border-left: 3px transparent;";
        }
        if (rbCt.checked)
        {
            inWorldVoiceSession.EnableAudioReceiving();
            inWorldVoiceSession.EnableAudioSending();
            transmissionType.styleSheet = "border-image: url(./data/assets/status_online.png) 3 3 3 3; border-top: 3px transparent; border-bottom: 3px transparent; border-right: 3px transparent; border-left: 3px transparent;";
        }
    }

    function getParticipants()
    {
        if (inWorldVoiceSession)
        {
            clearParticipants();

            var participants = inWorldVoiceSession.GetParticipantsNames();

            var pArray = new Array(); 
            pArray = participants.toString().split(",");

            var i;
            var rowCount = plTableWidget.rowCount;

            var muteButton = new Array();
            var buttonGroup = new QButtonGroup;

            for (i = 0; i < pArray.length; i++)
            {
                var p = new QTableWidgetItem(pArray[i]);
                plTableWidget.insertRow(rowCount + i);
                plTableWidget.setItem(rowCount + i, 0, p);
                muteButton[i] = new QPushButton("Mute");
                //muteButton[i].styleSheet = "border-image: url(./data/assets/mute.png) 3 3 3 3; border-top: 3px transparent; border-bottom: 3px transparent; border-right: 3px transparent; border-left: 3px transparent;";

                muteButton[i].stylesheet = "border: 2px solid #8f8f91; border-radius: 6px; background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #f6f7fa, stop: 1 #dadbde; min-width: 80px;";
                plTableWidget.setCellWidget(rowCount + i, 1, muteButton[i]);
                //plTableWidget.setRowHeight(rowCount + i, 70);
                buttonGroup.addButton(muteButton[i], rowCount+i);
            }
            buttonGroup['buttonClicked(int)'].connect(muteParticipant);

            pw.show(); 
        }
    }

    function keyPressed(event)
    {
        if (event.sequence.toString() == "P")
        {
            getParticipants();
        }
    }

    function muteParticipant(row) 
    {
        if (inWorldVoiceSession)
        {
            var participant = plTableWidget.item(row, 0).text();
            //print("Muting participant: " + participant + "\n");
            var muteButton = plTableWidget.cellWidget(row, 1);
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

    function showTransmissionWidget()
    {
        if (inWorldVoiceSession)
        {
            tm.show();
        }
    }

    function clearParticipants()
    {
        for (var i = plTableWidget.rowCount-1; i >= 0; --i)
        {
            plTableWidget.removeRow(i);
        }
    }

    function participantListChanged(participant)
    {
        updatePNLabel();
        
        if (pw.visible)
        {
            getParticipants();
        }
    }

    function channelListChanged(list)
    {
        channelList.clear();
        channelList.addItems(list);
    }

    function updatePNLabel()
    {
        if (inWorldVoiceSession)
        {
            var participants = inWorldVoiceSession.GetParticipantsNames();
            var pArray = new Array(); 
            pArray = participants.toString().split(",");
            numberOfParticipants.text = pArray.length.toString(); 
        }
    }

    input.TopLevelInputContext().KeyPressed.connect(keyPressed);
    inWorldVoiceSession.ParticipantJoined.connect(participantListChanged);
    inWorldVoiceSession.ParticipantLeft.connect(participantListChanged);
    inWorldVoiceSession.ChannelListChanged.connect(channelListChanged);
}

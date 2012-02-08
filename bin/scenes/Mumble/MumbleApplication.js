
// ***********************************************************************************************************************************
// Simple example script for using MumblePlugin as VOIP communications provider. Mumble can do VOIP over SSL TCP and encrypted UDP,
// in addition you can do text messaging with the current channel and privately. This example demonstrates most of the features
// provided with the MumblePlugin but not quite all of them, inspect the documentation for more information.
// Note that the 'Open Audio Wizard' widget is not implemented here in javascript but in the C++ code of MumblePlugin.
//
// 1. Download the Mumble installer that includes the Murmur server from http://www.mumble.com/ 
//    Mumble version should be >=1.2.3. Lower versions may work depending if they have celt >=0.11.1 support.
// 2. Install Mumble. Remember to check "Install Murmur server" in the install process,
//    the Murmur is not a part of the default install options.
// 3. Run Tundra with the example scene: Tundra --config viewer-browser.xml --console --file scenes/Mumble/scene.txml
//    This will start a client with the browser ui, but don't worry this script should hide the ui. 
//    If not you can use the menu Browser -> Hide Browser UI to see the scene.
// ***********************************************************************************************************************************

// Tell the script parser we depend on the following assets
// !ref: local://MumbleClientWidget.ui, QtUiFile
// !ref: local://MumbleConnectWidget.ui, QtUiFile
// !ref: local://voice-active.png
// !ref: local://voice-inactive.png

// These are just default that are applied to the connection widget on first show.
// You don't have to edit these if you want to use the client UI to setup connection info.
var _connectionInfo =
{
    host      : "127.0.0.1",    // Change to IP if you want to test remote Murmur servers.
    port      : 64738,          // Default port for Murmur, see murmur.ini for changing this.
    password  : "",             // Default password for Murmur is empty, see murmur.ini for changing this.
    channel   : "",             // Default Murmur server will have one channel called "Root". Empty channel name is depicted as "Root" when connecting via MumblePlugin.
    outputMuted : false,        // True means your voice is sent after connecting, false means your output is muted.
    intputMuted : false         // True means voice should be sent to us from other client after connecting, false means server wont send us the voice packets.
};

// Global variables
var _widget = null;
var _connectWidget = null;
var _buttonWizard = null;
var _buttonConnect = null;
var _buttonDisconnect = null;
var _buttonSelfMute = null;
var _buttonSelfDeaf = null;
var _buttonMuteSelected = null;
var _userList = null;
var _chatLog = null;
var _chatLine = null;
var _iconActive = null;
var _iconInactive = null;

function StartClient()
{
    // Hook to MumblePlugin
    mumble.Connected.connect(OnConnected);
    mumble.Disconnected.connect(OnDisconnected);
    mumble.ConnectionRejected.connect(OnRejected);

    mumble.MeCreated.connect(OnMeCreated);
    mumble.JoinedChannel.connect(OnJoinedChannel);

    mumble.UserMuted.connect(OnUserLocalMuteChanged);
    mumble.UserSelfMuted.connect(OnUserSelfMutedChange);
    mumble.UserSelfDeaf.connect(OnUserSelfDeafChange);
    mumble.UserSpeaking.connect(OnUserSpeakingChange);
    mumble.UserPositionalChanged.connect(OnUserPositionalChange);
    mumble.ChannelTextMessageReceived.connect(OnChannelTextMessageReceived);

    InitIcons();

    // Init UI
    _widget = ui.LoadFromFile("local://MumbleClientWidget.ui");
    _widget.visible = true;
    _widget.pos = new QPoint(15,240);

    _buttonConnect = findChild(_widget, "buttonOpenConnect");
    _buttonDisconnect = findChild(_widget, "buttonDisconnect");
    _buttonWizard = findChild(_widget, "buttonOpenWizard");
    _buttonSelfMute = findChild(_widget, "muteSelfToggle");
    _buttonSelfDeaf = findChild(_widget, "deafSelfToggle");
    _buttonMuteSelected = findChild(_widget, "muteSelectedToggle");
    _userList = findChild(_widget, "listUsers");
    _chatLog = findChild(_widget, "chatTextEdit");;
    _chatLine = findChild(_widget, "chatLineEdit");;

    _buttonConnect.clicked.connect(ShowConnectDialog);
    _buttonDisconnect.clicked.connect(mumble, mumble.Disconnect); // Direct connection to MumblePlugin C++ QObject
    _buttonWizard.clicked.connect(mumble, mumble.RunAudioWizard); // Direct connection to MumblePlugin C++ QObject
    _buttonSelfMute.clicked.connect(OnSelfMuteToggle);
    _buttonSelfDeaf.clicked.connect(OnSelfDeafToggle);
    _buttonMuteSelected.clicked.connect(OnMuteSelectedToggle);
    _userList.currentItemChanged.connect(OnUserSelected);
    _chatLine.returnPressed.connect(SendTextMessage);

    // Init state
    SetConnectionState(false, "Disconnected");
    SetChannelName("");
}

function InitIcons()
{
    // Just for reference: If the ref is http:// ref it will be found from cache,
    // if it is a local we need to query IAsset::DiskSource(). Usually in your app you will know
    // which one it is and just call the correct place directly.
    try
    {    
        var imageActivePath = asset.GetAssetCache().FindInCache("local://voice-active.png");
        if (imageActivePath == "")
            imageActivePath = asset.GetAsset("local://voice-active.png").DiskSource();
        var imageInactivePath = asset.GetAssetCache().FindInCache("local://voice-inactive.png");
        if (imageInactivePath == "")
            imageInactivePath = asset.GetAsset("local://voice-inactive.png").DiskSource();
    }
    catch (e)
    {
        imageActivePath = "";
        imageInactivePath = "";
    }

    _iconActive = new QIcon(new QPixmap(imageActivePath));
    _iconInactive = new QIcon(new QPixmap(imageInactivePath));
}

function SetConnectionState(connected, strState)
{
    var stateLabel = findChild(_widget, "labelConnectionState");
    stateLabel.text = strState;

    _buttonConnect.enabled = !connected;
    _buttonDisconnect.enabled = connected;
    _buttonWizard.enabled = connected;
    _buttonSelfMute.enabled = connected;
    _buttonSelfDeaf.enabled = connected;
    _buttonMuteSelected.enabled = connected;
    if (!connected)
        _buttonMuteSelected.text = "";
    _chatLine.enabled = connected;
    _chatLine.text = "";
    _chatLog.clear();
    _userList.clear();
}

function SetChannelName(channelName)
{
    var channelLabel = findChild(_widget, "labelChannelName");
    channelLabel.text = channelName;
}

function ShowConnectDialog()
{
    // Initialize if not yet done
    if (_connectWidget == null)
    {
        _connectWidget = ui.LoadFromFile("local://MumbleConnectWidget.ui");
        
        var widgets = GetConnectionDataWidgets(_connectWidget);
        widgets.connectButton.clicked.connect(Connect);
        widgets.cancelButton.clicked.connect(_connectWidget, _connectWidget.hide);

        widgets.host.text = _connectionInfo.host;
        widgets.port.value = _connectionInfo.port;
        widgets.password.text = _connectionInfo.password;
        widgets.channel.text = _connectionInfo.channel;
        if (client != null && client.GetLoginProperty("username") != "")
            widgets.username.text = client.GetLoginProperty("username");
        else
            widgets.username.text = "MumbleTestUser";
    }
    
    var mainWidgetRect = _widget.frameGeometry;
    _connectWidget.minimumHeight = _widget.height;
    _connectWidget.pos = new QPoint(mainWidgetRect.topRight().x() + 25, mainWidgetRect.topRight().y());
    _connectWidget.visible = true;
}

function GetConnectionDataWidgets(widget)
{
    if (widget == null)
        return null;

    var widgets =
    {
        host : findChild(widget, "hostLineEdit"),
        port : findChild(widget, "portSpinBox"),
        username : findChild(widget, "usernameLineEdit"),
        password : findChild(widget, "passwordLineEdit"),
        channel : findChild(widget, "channelLineEdit"),
        connectButton : findChild(widget, "buttonConnect"),
        cancelButton : findChild(widget, "buttonCancel")
    };

    return widgets;
}

function Connect()
{
    var widgets = GetConnectionDataWidgets(_connectWidget);
    mumble.Connect(widgets.host.text, widgets.port.value, widgets.username.text, widgets.password.text, widgets.channel.text, _connectionInfo.outputMuted, _connectionInfo.intputMuted);

    _connectWidget.visible = false;
    SetConnectionState(false, "Connecting to " + widgets.host.text + ":" + widgets.port.value.toString() + "...");  
}

function OnRejected(rejectType, reason)
{
    // enum RejectReasonWrongServerPW
    if (rejectType.value == 4)
        QMessageBox.warning(ui.MainWindow(), "", reason);
    // enum RejectReasonServerFull
    else if (rejectType.value == 6)
        QMessageBox.warning(ui.MainWindow(), "", "Server is full");
}

function OnConnected(host, port, username)
{
    SetConnectionState(true, "Connected to " + host + ":" + port.toString() + " as " + username);
}

function OnDisconnected(reason)
{
    if (reason != "")
        SetConnectionState(false, "Disconnected: " + reason);
    else
        SetConnectionState(false, "Disconnected");
}

function OnMeCreated(mumbleMe)
{
    // Can hook to own user ptr signals here.
    // Depends if you want to use own functions for processing 'me' signals.
    // Or a generic function to handle all like this example script shows.
    // You can use the signaling model you feel is best for you.
}

function OnJoinedChannel(mumbleChannel)
{
    SetChannelName(mumbleChannel.fullName);

    mumbleChannel.UserJoined.connect(OnUserJoinedPresentChannel);
    mumbleChannel.UserLeft.connect(OnUserLeftPresentChannel);
}

function GetUser(userId)
{
    var user = null;
    for(var i=0; i<_userList.count; ++i)
    {
        var item = _userList.item(i);
        if (item != null && item.data(Qt.UserRole) == userId)
        {
            user = { listItem: item, row: i };
            break;
        }
    }
    return user;
}

function OnUserJoinedPresentChannel(user)
{
    var listItem = new QListWidgetItem(_iconInactive, user.name + " (" + user.id.toString() + ")");
    listItem.setData(Qt.UserRole, user.id);

    if (user.isMe)
    {
        var font = listItem.font();
        font.setBold(true);
        listItem.setFont(font);
    }

    _userList.addItem(listItem);
}

function OnUserLeftPresentChannel(userId)
{
    var listItem = GetUser(userId);
    if (listItem != null)
        _userList.takeAt(listItem.row);
}

function OnUserSelected(listItem)
{
    if (listItem == null)
        listItem = _userList.currentItem();
    if (listItem != null && listItem.data(Qt.UserRole) != null)
    {
        var mumbleUser = mumble.User(listItem.data(Qt.UserRole));
        if (mumbleUser.isMe)
        {
            _buttonMuteSelected.enabled = false;
            _buttonMuteSelected.text = "";
        }
        else
        {
            _buttonMuteSelected.enabled = true;
            _buttonMuteSelected.text = mumbleUser.isMuted ? "Unmute " + mumbleUser.name : "Mute " + mumbleUser.name;
        }
    }
}

function OnSelfMuteToggle()
{
    var mumbleMe = mumble.Me();
    if (mumbleMe != null)
        mumble.SetOutputAudioMuted(!mumbleMe.isSelfMuted);
}

function OnSelfDeafToggle()
{
    var mumbleMe = mumble.Me();
    if (mumbleMe != null)
        mumble.SetInputAudioMuted(!mumbleMe.isSelfDeaf);
}

function OnMuteSelectedToggle()
{
    var currentItem = _userList.currentItem();
    if (currentItem != null && currentItem.data(Qt.UserRole) != null)
    {
        var user = mumble.User(currentItem.data(Qt.UserRole));
        if (user != null && !user.isMe)
            user.isMuted = !user.isMuted;
    }
}

function UpdateUserState(mumbleUser)
{
    var iter = GetUser(mumbleUser.id);
    if (iter != null)
    {
        var text = mumbleUser.name + " (" + mumbleUser.id.toString() + ")";
        var props = [];
        if (mumbleUser.isMuted)
            props.push("muted");
        if (mumbleUser.isSelfMuted)
            props.push("self muted");
        if (mumbleUser.isSelfDeaf)
            props.push("deaf");
        if (!mumbleUser.isPositional)
            props.push("non-positional");
        if (props.length > 0)
            text += " [" + props.join(", ") + "]";
        iter.listItem.setText(text);

        OnUserSelected(iter.listItem);
    }
}

function OnUserLocalMuteChanged(mumbleUser, isMuted)
{
    UpdateUserState(mumbleUser);
}

function OnUserSelfMutedChange(mumbleUser, isMuted)
{
    UpdateUserState(mumbleUser);
    if (mumbleUser.isMe)
        _buttonSelfMute.text = mumbleUser.isSelfMuted ? "Unmute Self" : "Mute self";
}

function OnUserSelfDeafChange(mumbleUser, isDeaf)
{
    UpdateUserState(mumbleUser);
    if (mumbleUser.isMe)
        _buttonSelfDeaf.text = mumbleUser.isSelfDeaf ? "Unmute Everyone" : "Mute Everyone";
}

function OnUserPositionalChange(mumbleUser, isPositional)
{
    UpdateUserState(mumbleUser);
}

function OnUserSpeakingChange(mumbleUser, speaking)
{
    var iter = GetUser(mumbleUser.id);
    if (iter != null)
        iter.listItem.setIcon(speaking ? _iconActive : _iconInactive);
}

function SendTextMessage()
{
    var message = _chatLine.text;
    if (message != "")
    {
        // Own messages are of course not relayed back to us, add by hand.
        OnChannelTextMessageReceived(mumble.Me(), message);
        // Send message to current channel.
        mumble.SendTextMessage(message);
        _chatLine.text = "";
    }
}

function OnChannelTextMessageReceived(mumbleUser, message)
{
    if (message != "")
        _chatLog.appendHtml("[<span style=\"color:" + (mumbleUser.isMe == true ? "red" : "blue") + ";\">" + mumbleUser.name + "</span>] " + message);
}

// Bootstrap and utility functions

function OnScriptDestroyed()
{
    mumble.Disconnect("Client closed");

    if (_widget != null)
    {
        _widget.visible = false;
        ui.RemoveWidgetFromScene(_widget);
        _widget = null;
    }
    if (_connectWidget != null)
    {
        _connectWidget.visible = false;
        ui.RemoveWidgetFromScene(_connectWidget);
        _connectWidget = null;
    }    
}

function LogInfo(msg)  { console.LogInfo("[MumbleApplication]: " + msg);  }
function LogError(msg) { console.LogError("[MumbleApplication]: " + msg); }

if (!server.IsRunning() && !framework.IsHeadless())
{
    if (framework.GetModuleByName("MumblePlugin") != null)
    {
        try
        {
            // Hide browser ui
            if (browserplugin != null)
                browserplugin.SetBrowserVisibility(false);
        }
        catch (e) {}

        engine.ImportExtension("qt.core");
        engine.ImportExtension("qt.gui");

        StartClient();
    }
    else
        console.LogError("MumblePlugin not preset, its not in your startup config?");
}

// !ref: local://ChatWidget.ui
// !ref: local://JoinWidget.ui
// !ref: local://UserList.ui
// !ref: local://PrivateChatWidget.ui

engine.ImportExtension("qt.core");
engine.ImportExtension("qt.gui");
engine.ImportExtension("qt.uitools");

//Server side of the ChatApplication
//Server shows the ChatWidget, but it's not able to use it to send own messages
function ServerControl()
{
  me.Action("ClientSendMessage").Triggered.connect(this, this.ClientMessage);
  me.Action("ClientSendPrivateMessage").Triggered.connect(this, this.PrivateClientMessage);
  me.Action("ServerUpdateUserList").Triggered.connect(this, this.ServerUpdateUserList);
  server.UserConnected.connect(this, this.UserConnected);
  server.UserDisconnected.connect(this, this.UserDisconnected);
}

function GetClientUsername(connection)
{
  var username = connection.GetProperty("username");
  if (username.length == 0)
    username = "Unnamed user";
  return username;
}

ServerControl.prototype.UserConnected = function(cid, connection)
{
    var msg = GetClientUsername(connection) + " connected.";
    me.Exec(4, "ServerSendMessage", msg);
}

ServerControl.prototype.UserDisconnected = function(cid, connection)
{
    var msg = GetClientUsername(connection) + " disconnected.";
    
    me.Exec(4, "ServerSendMessage", msg);
    me.Exec(4, "RemoveUserFromList", GetClientUsername(connection));
}

//Receive incoming messages from client 
//and pass it to all clients
ServerControl.prototype.ClientMessage = function(sender, msg)
{
    if (msg.length > 0)
    {
        var message = (sender + ": " + msg);
        me.Exec(4, "ServerSendMessage", message);
    }
}

ServerControl.prototype.PrivateClientMessage = function(sender, receiver, msg)
{
    if (msg.length > 0)
    {
        var userIDs = new Array();
        userIDs = server.GetConnectionIDs();
        for(var i = 0; i < userIDs.length; i++)
        {
            if(server.GetUserConnection(userIDs[i]).GetProperty("username") == receiver)
            {
                server.GetUserConnection(userIDs[i]).Exec(scene.GetEntityByName("ChatApplication"), "ServerSendPrivateMessage", sender + ": " + msg);
            }    
            
        }
    }
}

ServerControl.prototype.ServerUpdateUserList = function(user)
{
    me.Exec(4, "UpdateUserList", user);
}

//Client side of the ChatApplication
function ClientControl(userName)
{
    this.name = userName;
    lineEdit.returnPressed.connect(this, this.SendMessage);
    buttonToggleLog.clicked.connect(this, this.ToggleLog);
    me.Action("ServerSendMessage").Triggered.connect(this, this.ReceiveServerMessage);
    me.Action("ServerSendPrivateMessage").Triggered.connect(this, this.ReceivePrivateServerMessage)
    me.Action("NewUserConnected").Triggered.connect(this, this.NewUserConnected);
    me.Action("UpdateUserList").Triggered.connect(this, this.UpdateUserList);
    me.Action("RemoveUserFromList").Triggered.connect(this, this.RemoveUserFromList);
    
    buttonUserList.clicked.connect(this, this.ToggleUserList);
    userListWidget.itemDoubleClicked.connect(this, this.StartPrivateChat)
   
    this.hoveringText = null;
    this.showTextTime = 4;

    if (this.hoveringText == null) {
        var name = "Avatar" + client.GetConnectionID();
        var entity = scene.GetEntityByName(name);
        if (entity != null) {
            this.hoveringText = entity.CreateComponent("EC_HoveringText");
            this.hoveringText.text = " ";
            
            // Move it over a avatar.
            var pos = this.hoveringText.position;
            pos.y = 2.0;
      
            this.hoveringText.position = pos;
            
            
        }
        else {
            print("Error: Avatar not found");
            return;
        }


    }
    
}
ClientControl.prototype.HideText = function() {
    
    if (this.hoveringText != null) {
        this.hoveringText.text = " ";
    }
}



ClientControl.prototype.SendMessage = function() {
    // Try to be flexible: If we specified a username field in the login
    // properties when connecting, use that field as the username. Otherwise,
    // we show the login screen and ask the username from there.
    var name = client.GetLoginProperty("username");
    if (name.length == 0)
        name = this.name;

    var msg = lineEdit.text;
    me.Exec(2, "ClientSendMessage", client.GetLoginProperty("username"), msg);
    lineEdit.text = "";
    this.hoveringText.text = msg;
    frame.DelayedExecute(this.showTextTime).Triggered.connect(this, this.HideText);
}


ClientControl.prototype.ReceiveServerMessage = function(msg)
{
    chatLog.append(msg);
}

ClientControl.prototype.SendPrivateMessage = function() {
    //var line = findChild(this.widget.widget(), "lineEdit");
    var line = findChild(this.widget, "lineEdit");
    //var ownPrivateLog = findChild(this.widget.widget(), "privateChatLog");
    var ownPrivateLog = findChild(this.widget, "privateChatLog");
    var msg = line.text;
    if (msg.length > 0) {
        ownPrivateLog.append("me: " + msg);
        me.Exec(2, "ClientSendPrivateMessage", client.GetLoginProperty("username"), this.widget.windowTitle, msg);
        line.text = "";
    }
}

ClientControl.prototype.ReceivePrivateServerMessage = function(msg)
{
    var privateChatLog = null;
    var msgArray = msg.split(":");
    this.OpenPrivateChatWidget(msgArray[0]);
    
    for(var i = 0; i < openChats.length; i++)
    {
        if(openChats[i] == msgArray[0])
        {
            privateChatLog = findChild(privateChatWidgets[i], "privateChatLog");
        }
    }
    privateChatLog.append(msg);
}

ClientControl.prototype.NewUserConnected = function(msg)
{
    userListWidget.addItem(msg);
    me.Exec(2, "ServerUpdateUserList", client.GetLoginProperty("username"));
}

ClientControl.prototype.UpdateUserList = function(user)
{
    var hasUser = false;
    for(var i = 0; i < userListWidget.count; i++)
    {
        if(userListWidget.item(i).text() == user || client.GetLoginProperty("username") == user )
        {
            hasUser = true;
        }
    }    
    
    if(!hasUser)
    {
        userListWidget.addItem(user);
    }
}

ClientControl.prototype.RemoveUserFromList = function(user)
{
    var hasUser = false;
    var index = -1;
    for(var i = 0; i < userListWidget.count; i++)
    {
        if(userListWidget.item(i).text() == user)
        {
            hasUser = true;
            index = i;
        }
    }
    
    if(hasUser)
    {
        delete userListWidget.takeItem(index);
    }
}

ClientControl.prototype.ToggleLog = function()
{
    log_visible = !log_visible;
    chatLog.setVisible(log_visible);
    if (log_visible)
    {
        buttonToggleLog.text = "Hide Log";
        widget.setFixedSize(350,200);
    }
    else
    {
        buttonToggleLog.text = "Show Log";
        widget.setFixedSize(350,30);
    }
    proxy.x = 5;
    proxy.y = 25;
}

ClientControl.prototype.ToggleUserList = function()
{
    userlist_visible = !userlist_visible;
    listProxy.visible = userlist_visible;
}

ClientControl.prototype.StartPrivateChat = function(user)
{
    this.OpenPrivateChatWidget(user.text());
}

ClientControl.prototype.PrivateChatClosed = function()
{
    var inTheArray = false;
    var removeIndex;
    for(var i = 0; i < openChats.length; i++)
    {
        if(openChats[i] == this.widget.windowTitle)
        {
            inTheArray = true;
            removeIndex = i;
        }
    }
    
    if(inTheArray)
    {
        openChats.splice(removeIndex, 1);
        privateChatWidgets.splice(removeIndex, 1);
        privateChatProxies.splice(removeIndex, 1);
    }
}

ClientControl.prototype.FocusChanged = function(widget, pos)
{
    this.widget = widget;
}

ClientControl.prototype.OpenPrivateChatWidget = function(userStr)
{
    var hasUser = false;
    var index = -1;
    
    for(var i = 0; i < openChats.length; i++)
    {
        if(openChats[i] == userStr)
        {
            hasUser = true;
            index = i;
        }
    }

    if(!hasUser)
    {
        var privateChatWidget = ui.LoadFromFile("local://PrivateChatWidget.ui", false);
        privateChatWidget.setWindowTitle(userStr);
        var buttonSend = findChild(privateChatWidget, "buttonSend");
        var lEdit = findChild(privateChatWidget, "lineEdit");
        
        this.widget = privateChatWidget;
        
        lEdit.returnPressed.connect(this, this.SendPrivateMessage);
        buttonSend.clicked.connect(this, this.SendPrivateMessage);
        
        openChats.unshift(userStr);
        privateChatWidgets.unshift(privateChatWidget);
        privateChatProxies.unshift(new UiProxyWidget(privateChatWidget));
        
        privateChatProxies[0].Closed.connect(this, this.PrivateChatClosed);
        
        ui.AddProxyWidgetToScene(privateChatProxies[0]);
        privateChatProxies[0].x = Math.floor(Math.random() * 301);
        privateChatProxies[0].y = 300;
        privateChatProxies[0].visible = true;
        
        //privateChatProxies[0].ProxyUngrabbed.connect(this, this.FocusChanged);
    }
}





var chatControl;
var users;

if (server.IsRunning())
{
    print("Starting Chat Server");
    chatControl = new ServerControl();
}
else
{    
    print("Starting Chat Client");
    
    // Ui only for client
    var widget = ui.LoadFromFile("local://ChatWidget.ui", false);
    var chatLog = findChild(widget, "chatLog");
    var lineEdit = findChild(widget, "lineEdit");
    var buttonToggleLog = findChild(widget, "buttonToggleLog");
    var buttonUserList = findChild(widget, "buttonUserList");
    var proxy = new UiProxyWidget(widget);

    ui.AddProxyWidgetToScene(proxy);
    proxy.visible = true;
    proxy.windowFlags = 0;
    
    var log_visible = true;
    var userlist_visible = true;
    
    var listWidget = ui.LoadFromFile("local://UserList.ui", false);
    var userListWidget = findChild(listWidget, "userListWidget");
    var listProxy = new UiProxyWidget(listWidget);
    
    ui.AddProxyWidgetToScene(listProxy);
    listProxy.visible = userlist_visible;
    listProxy.windowFlags = 0;
    
    listProxy.x = 360;
    listProxy.y = 25;
    
    var privateChatWidgets = new Array();
    var privateChatProxies = new Array();
    var openChats = new Array();

    // If the login property specifies a username, don't show the username field.
    // Otherwise, before joining the chat, first ask the user for a username.
    
    // Try to be flexible: If we specified a username field in the login
    // properties when connecting, use that field as the username. Otherwise,
    // we show the login screen and ask the username from there.
    var username = client.GetLoginProperty("username");
    if (username.length == 0)
    {
        var joinWidget = ui.LoadFromFile("local://JoinWidget.ui", false);
        var btn = findChild(joinWidget, "pushJoin");
        var lineUser = findChild(joinWidget, "lineUserName");

        var joinProxy = new UiProxyWidget(joinWidget);
        ui.AddProxyWidgetToScene(joinProxy);
        joinProxy.x = 50;
        joinProxy.y = 50;
        joinProxy.visible = true;
        joinProxy.windowFlags = 0;

        btn.pressed.connect(CreateUser);
        lineUser.returnPressed.connect(CreateUser);
    }
    else // The user specified a username. Use that directly.
    {
        proxy.visible = true;
        chatControl = new ClientControl(username);
        chatControl.ToggleLog();
    }
    me.Exec(4, "NewUserConnected", client.GetLoginProperty("username"));
    
    
}

function CreateUser()
{
    userName = lineUser.text;
    joinProxy.visible = false;
    proxy.visible = true;
    chatControl = new ClientControl(userName);
}






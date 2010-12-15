engine.ImportExtension("qt.core");

//Server side of the ChatApplication
//Server shows the ChatWidget, but it's not able to use it to send own messages
function ServerControl()
{
	me.Action("ClientSendMessage").Triggered.connect(this, this.ClientMessage);
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
  chatLog.append(msg);
	me.Exec(4, "ServerSendMessage", msg);
}

ServerControl.prototype.UserDisconnected = function(cid, connection)
{
  var msg = GetClientUsername(connection) + " disconnected.";      
  chatLog.append(msg);
	me.Exec(4, "ServerSendMessage", msg);
}

//Receive incoming messages from client 
//and pass it to all clients
ServerControl.prototype.ClientMessage = function(sender, msg)
{
  if (msg.length > 0)
  {
  	var message = (sender + ": " + msg);
  	chatLog.append(message);
  	me.Exec(4, "ServerSendMessage", message);
  }
}

//Client side of the ChatApplication
function ClientControl(userName)
{
	this.name = userName;
	lineEdit.returnPressed.connect(this, this.SendMessage);
	me.Action("ServerSendMessage").Triggered.connect(this, this.ReceiveServerMessage);
}

ClientControl.prototype.SendMessage = function()
{
  // Try to be flexible: If we specified a username field in the login
  // properties when connecting, use that field as the username. Otherwise,
  // we show the login screen and ask the username from there.
  var name = client.GetLoginProperty("username");
  if (name.length == 0)
     name = this.name;
     
	var msg = lineEdit.text;
	me.Exec(2, "ClientSendMessage", client.GetLoginProperty("username"), msg);	
	lineEdit.text = "";
}

ClientControl.prototype.ReceiveServerMessage = function(msg)
{
	chatLog.append(msg);
}

print("Loading ChatApplication");

var widget = uiservice.LoadFromFile(".\\scenes\\ChatApplication\\ChatWidget.ui", false);	
var chatLog = findChild(widget, "chatLog");
var lineEdit = findChild(widget, "lineEdit");
var proxy = new UiProxyWidget(widget);
uiservice.AddWidgetToScene(proxy);
proxy.x = 10;
proxy.y = 10;
proxy.visible = false;
proxy.windowFlags = 0;

var chatControl;
serverComponent = me.GetComponentRaw("EC_DynamicComponent", "server");

if (serverComponent)
{
	print("Starting Chat Server");
	chatControl = new ServerControl();
	proxy.visible = true;
}
else
{	
	print("Starting Chat Client");
	
	// If the login property specifies a username, don't show the username field.
	// Otherwise, before joining the chat, first ask the user for a username.
	
  // Try to be flexible: If we specified a username field in the login
  // properties when connecting, use that field as the username. Otherwise,
  // we show the login screen and ask the username from there.
  var username = client.GetLoginProperty("username");
  if (username.length == 0)
  {
  	var joinWidget = uiservice.LoadFromFile(".\\scenes\\ChatApplication\\JoinWidget.ui", false);	
  	var btn = findChild(joinWidget, "pushJoin");
  	var lineUser = findChild(joinWidget, "lineUserName");
  
  	var joinProxy = new UiProxyWidget(joinWidget);
  	uiservice.AddWidgetToScene(joinProxy);
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
  }
}

function CreateUser()
{
	userName = lineUser.text;
	joinProxy.visible = false;
	proxy.visible = true;
	chatControl = new ClientControl(userName);
}

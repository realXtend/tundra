//Server side of the ChatApplication
//Server shows the ChatWidget, but it's not able to use it to send own messages
function ServerControl ()
{
	me.Action("ClientSendMessage").Triggered.connect(this, this.ClientMessage);
	me.Action("ServerSendMessage").Triggered.connect(empty); //Need to register action on Server, but Client is the only one to handle this action
}

//Receive incoming messages from client 
//and pass it to all clients
ServerControl.prototype.ClientMessage = function(sender, msg)
{
	var message = (sender + " : " + msg);
	chatLog.append(message);
	me.Exec(4, "ServerSendMessage", message);
}


//Client side of the ChatApplication
function ClientControl (userName)
{
	this.name = userName;
	lineEdit.returnPressed.connect(this, this.SendMessage);
	me.Action("ServerSendMessage").Triggered.connect(this, this.ReceiveServerMessage);
	me.Exec(2, "ClientSendMessage", this.name, "Joined Chat");	

}

ClientControl.prototype.SendMessage = function()
{
	var msg = lineEdit.text;
	me.Exec(2, "ClientSendMessage", this.name, msg);	
	lineEdit.text = "";
}

ClientControl.prototype.ReceiveServerMessage = function(msg)
{
	chatLog.append(msg);
}

function empty()
{
}

print("Loading ChatApplication");

var widget = uiservice.LoadFromFile(".\\scenes\\ChatApplication\\ChatWidget.ui", false);	
var chatLog = widget.findChild("chatLog");
var lineEdit = widget.findChild("lineEdit");
var proxy = new UiProxyWidget(widget);
uiservice.AddWidgetToScene(proxy);
proxy.x = 10;
proxy.y = 10;
proxy.visible = false;
proxy.windowFlags = 0;

var chatControl;
server = me.GetComponentRaw("EC_DynamicComponent", "server");

if (server)
{
	print("Starting Chat Server");
	chatControl = new ServerControl();
	proxy.visible = true;
}
else
{	
	print("Starting Chat Client");
	
	//Insert Username from widget. In future this is replaced
	var joinWidget = uiservice.LoadFromFile(".\\scenes\\ChatApplication\\JoinWidget.ui", false);	
	var btn = joinWidget.findChild("pushJoin");
	var lineUser = joinWidget.findChild("lineUserName");

	var joinProxy = new UiProxyWidget(joinWidget);
	uiservice.AddWidgetToScene(joinProxy);
	joinProxy.x = 50;
	joinProxy.y = 50;
	joinProxy.visible = true;
	joinProxy.windowFlags = 0;

	btn.pressed.connect(CreateUser);
	lineUser.returnPressed.connect(CreateUser);

	me.Action("ClientSendMessage").Triggered.connect(empty);  //Need to register action on Client, but Server is the only one to handle this action
}

function CreateUser()
{
	userName = lineUser.text;
	joinProxy.visible = false;
	proxy.visible = true;
	chatControl = new ClientControl(userName);
}

function OnScriptDestroyed()
{	
	print("Closing ChatApplication");
	
	if(!server)
	{
		me.Exec(2, "ClientSendMessage", chatControl.name, " has left from the chat");	
	}
}

/**
 *  Tundra xmpp chat application supporting Muc-chatrooms and private messages.
 *  Contains some semi ugly hacks to get around QtScript's shortcomings.
 *  Depends on Tundra XMPPModule.
 *
 *  How to use: Fill in your XMPP information into the script at the bottom to
 *  test the script. You can call Client constructor and Client.Connect with
 *  your XMPP infromation from another script similarly.
 *
 *  Copyright (c) 2012 CIE / University of Oulu, All Rights Reserved
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 */

engine.ImportExtension("qt.core");
engine.ImportExtension("qt.gui");

// Implements main dialog that contains controls for selecting individual chats.
function ChatDialog(muc)
{
    this.muc = muc;

    this.conversations = new Array();
    this.selectorButtons = new Array();

    this.dialog = new QDialog();
    this.dialog.setStyleSheet("QDialog {background-color: transparent;}");
    this.dialog.windowOpacity = 0.6;
    this.layout = new QVBoxLayout(this.dialog);

    this.chatSelectorLayout = new QHBoxLayout(this.dialog);

    this.layout.addLayout(this.chatSelectorLayout);

    this.proxy = new UiProxyWidget(this.dialog);

    ui.AddProxyWidgetToScene(this.proxy);
    this.proxy.x = 100;
    this.proxy.y = 100;
    this.proxy.windowFlags = 0;
    this.proxy.visible = true;
}

ChatDialog.prototype.AddConversation = function(jid, type)
{
    // Create new ConversationForm
    var conv = new ConversationForm(this.muc, this.dialog, this.layout, jid, "room");
    this.conversations.push(conv);

    // Create control button for this conversation
    var roomName = jid.split("@")[0];
    var button = { qbutton: new QPushButton(roomName),
                   active: false };

    button.qbutton.minimumHeight = 20;
    button.qbutton.setStyleSheet("QPushButton {color: #FF9340; background-color: #0A64A4; border: 1px solid black; padding: 0 3px 0 3px;}");

    // Hax, because QtScript. Take the spacer from the end, append button and insert a spacer again.
    this.chatSelectorLayout.takeAt(this.chatSelectorLayout.count() -1);
    this.chatSelectorLayout.addWidget(button.qbutton, 0, Qt.AlignLeft);
    this.chatSelectorLayout.insertStretch(-1, 400);
    this.selectorButtons.push(button);

    // Set as active conversation if no conversations are active yet.
    var activeConversation = this.GetActiveConversation();
    if(activeConversation === null) {
        this.SetActiveConversation(jid);
    }

    // Connect to SetActiveConversation with unique string per pushbutton
    button.qbutton.pressed.connect(this, function() { this.SetActiveConversation(jid) });

    return conv;
}

ChatDialog.prototype.SetActiveConversation = function(roomJid)
{
    var oldConversation = this.GetActiveConversation();
    var newConversation = this.GetConversation(roomJid);
    var oldSelector = this.GetActiveSelectorButton();
    var newSelector = this.GetSelectorButton(roomJid);

    if(oldConversation != null) {
        oldConversation.Hide();
    }
    newConversation.Show();

    if(oldSelector != null) {
        oldSelector.qbutton.setStyleSheet("QPushButton {color: #FF9340; background-color: #0A64A4; border: 1px solid black; padding: 0 3px 0 3px;}");
        oldSelector.active = false;
    }
    newSelector.qbutton.setStyleSheet("QPushButton {color: #0A64A4; background-color: #FF9340; border: 1px solid black; padding: 0 3px 0 3px;}");
    newSelector.active = true;
}

ChatDialog.prototype.GetActiveConversation = function()
{
    for(var i = 0; i < this.conversations.length; ++i) {
        if(this.conversations[i].visible === true) {
            return this.conversations[i];
        }
    }
    return null;
}

ChatDialog.prototype.GetConversation = function(roomJid)
{
    for(var i = 0; i < this.conversations.length; ++i) {
        if(this.conversations[i].jid === roomJid) {
            return this.conversations[i];
        }
    }
}

ChatDialog.prototype.GetActiveSelectorButton = function()
{
    for(var i = 0; i < this.selectorButtons.length; ++i) {
        if(this.selectorButtons[i].active === true) {
            return this.selectorButtons[i];
        }
    }
    return null;
}

ChatDialog.prototype.GetSelectorButton = function(roomJid)
{
    var roomName = roomJid.split("@")[0];
    for(var i = 0; i < this.selectorButtons.length; ++i) {
        if(this.selectorButtons[i].qbutton.text === roomName) {
            return this.selectorButtons[i];
        }
    }
}

// Implements a single conversation ("room" or "private") and ui elements related to it.
// Handles all signaling from XMPPModule related to conversations, for the sake of simplicity.
function ConversationForm(muc, dialog, layout, jid, type)
{
    this.muc = muc;
    this.dialog = dialog;
    this.layout = layout;
    this.jid = jid;
    this.type = type;
    this.visible = false;

    this.BuildDialog();

    this.muc.MessageReceived.connect(this, this.MessageReceived);
    this.muc.UserJoinedRoom.connect(this, this.UserJoined);
    this.muc.UserLeftRoom.connect(this, this.UserLeft);
    this.messageButton.clicked.connect(this, this.SendMessage);
}

ConversationForm.prototype.BuildDialog = function()
{
    this.chatAreaLayout = new QHBoxLayout(/*this.dialog*/);
    this.chatArea = new QListWidget();
    this.chatArea.wordWrap = true;
    this.chatArea.minimumWidth = 500;
    this.chatArea.minimumHeight = 200;
    this.chatArea.setStyleSheet("QListWidget {color: #FF9340; background-color: #0A64A4; border: 1px solid black;}");
    this.chatAreaLayout.addWidget(this.chatArea, 0, Qt.AlignLeft);

    this.participantArea = new QListWidget();
    this.participantArea.minimumWidth = 80;
    this.participantArea.maximumWidth = 80;
    this.participantArea.minimumHeight = 200;
    this.participantArea.setStyleSheet("QListWidget {color: #FF9340; background-color: #0A64A4; border: 1px solid black;}");
    this.chatAreaLayout.addWidget(this.participantArea, 0, Qt.AlignRight);

    //this.layout.addLayout(this.chatAreaLayout);

    this.messageEditLayout = new QHBoxLayout(/*this.dialog*/);

    this.messageEdit = new QLineEdit();
    this.messageEdit.minimumWidth = 500;
    this.messageEdit.minimumHeight = 20;
    this.messageEdit.setStyleSheet("QLineEdit {color: #FF9340; background-color: #0A64A4; border: 1px solid black;}");
    this.messageEditLayout.addWidget(this.messageEdit, 0, Qt.AlignLeft);

    this.messageButton = new QPushButton("Send");
    this.messageButton.minimumWidth = 80;
    this.messageButton.minimumHeight = 20;
    this.messageButton.setStyleSheet("QPushButton {color: #FF6F00; background-color: #03406A; border: 1px solid black;}");
    this.messageEditLayout.addWidget(this.messageButton, 0, Qt.AlignRight);

    //this.layout.addLayout(this.messageEditLayout);
}

ConversationForm.prototype.Hide = function()
{
    print("Hiding conversation: " + this.jid);
    this.layout.removeItem(this.chatAreaLayout);
    this.layout.removeItem(this.messageEditLayout);
    this.chatAreaLayout.setParent(null);
    this.messageEditLayout.setParent(null);
    this.visible = false;
}

ConversationForm.prototype.Show = function()
{
    print("Showing conversation: " + this.jid);
    this.layout.addLayout(this.chatAreaLayout);
    this.layout.addLayout(this.messageEditLayout);
    this.visible = true;
}

ConversationForm.prototype.SendMessage = function()
{
    print("Sending message to: " + this.jid);
    this.muc.SendMessage(this.jid, this.messageEdit.text);
    this.messageEdit.text = "";
}

ConversationForm.prototype.MessageReceived = function(roomJid, senderJid, message, type)
{
    if(roomJid != this.jid)
        return;

    sender = senderJid.split("/")[1];
    this.AddMessage(sender, message);
}

ConversationForm.prototype.UserJoined = function(roomJid, userJid)
{
    if(roomJid != this.jid)
        return;

    user = userJid.split("/")[1];
    this.AddMessage("**", user + " joined the room.");
    this.AddParticipant(user);
}

ConversationForm.prototype.UserLeft = function(roomJid, userJid)
{
    if(roomJid != this.jid)
        return;

    user = userJid.split("/")[1];
    this.AddMessage("**", user + " left the room.");
    this.RemoveParticipant(user);
}

ConversationForm.prototype.AddMessage = function(sender, message)
{
    var date = new Date();
    var fullMessage = "[" + date.getHours() + ":" + date.getMinutes() + ":" + date.getSeconds() + "] " + sender + ": " + message;
    this.chatArea.addItem(fullMessage);
}

ConversationForm.prototype.AddParticipant = function(participant)
{
    this.participantArea.addItem(participant)
}

ConversationForm.prototype.RemoveParticipant = function(participant)
{
    for(var i = 0; i < this.participantArea.count; ++i) {
        var item = this.participantArea.item(i);
        if(item.text() == participant) {
            this.participantArea.takeItem(i);
            return;
        }
    }
}

// Initializes the client. Connecting is done separately with Client.Connect.
function Client()
{
    var xmppModule = framework.GetModuleByName("XMPP");
    this.xmppClient = xmppModule.NewClient();

    this.muc = this.xmppClient.AddExtension("Muc");
    this.xmppClient.Connected.connect(this, this.JoinWebchat);

    this.dialog = new ChatDialog(this.muc);
}

// Attempts to connect to the XMPP server. The rooms parameter is an array
// of arrays of room name + password combination.
Client.prototype.Connect = function(username, serverUrl, password, rooms)
{
    this.username = username;
    this.rooms = rooms;
    this.jid = this.username + "@" + serverUrl;
    this.xmppClient.ConnectToServer(serverUrl, this.jid, password);
}

Client.prototype.JoinWebchat = function()
{
    this.muc.RoomAdded.connect(this, this.RoomJoined);
    for (var i = 0; i < this.rooms.length; i++)
        this.muc.JoinRoom(this.rooms[i][0], this.username, this.rooms[i][1]);
}

Client.prototype.RoomJoined = function(roomJid, nickname)
{
    var conversation = this.dialog.AddConversation(roomJid, "room");
    var userlist = this.muc.GetParticipants(roomJid);
    for(var i = 0; i < userlist.length; ++i) {
        var participant = userlist[i].split("/")[1];
        conversation.AddParticipant(participant);
    }
}

if(!server.IsRunning())
{
    var client = new Client();
    // Client.Connect takes an array of room name & password combination.
    // Fill these with your room information to test.
    var room1 = "";
    var room2 = "";
    var room1pass = "";
    var room2pass = "";
    var rooms = new Array([room1, room1pass], [room2, room2pass]);
    // Fill with credentials (username, server url, password and room info array)
    // to test.
    client.Connect("", "", "", rooms);
}

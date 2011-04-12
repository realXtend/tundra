"""an irc bridge

relays messages between in-world chat, and an irc channel.
notifies of users joining & leaving the scene on irc too.
"""

import naali
import circuits
import ircclient

from circuits.net.protocols import irc #IRC, PRIVMSG, USER, NICK, JOIN, Nick

IRCCHANNEL = "#realxtend-dev"
NICKNAME = "tundraserver"
IRCSERVER = "irc.freenode.net"
PORT = 6667

class ServerRelay(circuits.Component):
    channel = "ircclient"

    def __init__(self, nick=NICKNAME, ircchannel=IRCCHANNEL, host=IRCSERVER, port=PORT):
        if not naali.server.IsAboutToStart():
            return
        circuits.Component.__init__(self)
        self.client = ircclient.Client(nick, ircchannel, host, port)
        self += self.client #so that we can handle irc events here too
        self.client.connect()

        self.chatapp = None

        self.scene = None
        self.chatapp = None

        naali.server.connect("UserConnected(int, UserConnection*)", self.onUserConnected)
        naali.server.connect("UserDisconnected(int, UserConnection*)", self.onUserDisconnected)

        #when live reloading, chatapp is already there:
        self.scene = naali.getDefaultScene()
        if self.scene is not None:
            self.chatapp = self.scene.GetEntityByNameRaw("ChatApplication")
            self.init_chatapp_handlers()

        self.cmds = {
            'help': self.help,
            'users': self.users,
            'fps': self.fps
            }

    def help(self):
        msg = "i know: %s" % self.cmds.keys()
        self.say(msg)

    def users(self):
        msg = "i have %d users (client connections) in-world" % len(naali.server.GetConnectionIDs())
        self.say(msg)

    def fps(self):
        self.say("60fps, of course! (no, sorry, actually i didn't check")

    def init_chatapp_handlers(self):
        csm = self.chatapp.Action("ClientSendMessage")
        csm.connect("Triggered(QString, QString, QString, QStringList)", self.onClientSendMessage)

    #a naali event via circuits
    def on_sceneadded(self, name): #XXX port to SceneAPI SceneAdded signal
        if self.scene is not None: #XXX what to do with scene changes? aren't supposed to happen on server i guess?
            print "IRC: WARNING -- reiniting with new scene, not really written for scene changing though!"
        self.scene = naali.getScene(name)
        self.scene.connect("EntityCreated(Scene::Entity*, AttributeChange::Type)", self.onNewEntity)

    #a qt signal handler
    def onNewEntity(self, entity, changeType):
        #print entity.Name
        if entity.Name == "ChatApplication":
            self.chatapp = entity
            print "IRC: Found ChatApp!"
            self.scene.disconnect("EntityCreated(Scene::Entity*, AttributeChange::Type)", self.onNewEntity)
            self.init_chatapp_handlers()

    def onUserConnected(self, connid, user):
        self.note("New user connected: %s" % user.GetProperty("username"))

    def onUserDisconnected(self, connid, user):
        self.note("User %s disconnected." % user.GetProperty("username"))

    def onClientSendMessage(self, sender, msg):
        print "IRC onClientSendMessage:", sender, msg
        toirc = "[%s] %s" % (sender, msg)
        self.say(toirc)

    def say(self, msg):
        self.push(irc.PRIVMSG(IRCCHANNEL, msg))

    def note(self, msg):
        self.push(irc.NOTICE(IRCCHANNEL, msg))

    #a circuits event from the underlying irc client (self.client)
    def message(self, source, target, message):
        if target[0] == "#":
            s = "<%s:%s> %s" % (target, source[0], message)
        else:
            s = "-%s- %s" % (source, message)

        print "IRC:", s

        if self.chatapp is not None:
            self.chatapp.Exec(4, "ServerSendMessage", s)

        nick = self.client.nick
        #copy-paste from kbd -- consider just using kbd: https://bitbucket.org/prologic/kdb/src/0982b3f52af0/kdb/plugin.py
        print "MSG, NICK:", message, nick
        if message.startswith(nick):
            message = message[len(nick):]
            while len(message) > 0 and message[0] in [",", ":", " "]:
                message = message[1:]
                
            words = message.split()
            cmd = words[0]
            print "CMD", cmd
            #args = words[1:]
            if cmd in self.cmds:
                self.cmds[cmd]() #(args)
                

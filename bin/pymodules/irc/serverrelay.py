"""an irc bridge

relays messages between in-world chat, and an irc channel.
notifies of users joining & leaving the scene on irc too.
"""

import naali
import circuits
import ircclient

IRCCHANNEL = "#realxtend"
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

        #when live reloading, chatapp is already there:
        scene = naali.getDefaultScene()
        if scene:
            self.chatapp = scene.GetEntityByNameRaw("ChatApplication")

    #a naali event via circuits
    def on_sceneadded(self, name): #XXX port to SceneAPI SceneAdded signal
        self.scene = naali.getScene(name)
        self.scene.connect("EntityCreated(Scene::Entity*, AttributeChange::Type)", self.onNewEntity)

    #a qt signal handler
    def onNewEntity(self, entity, changeType):
        print entity.Name
        if entity.Name == "ChatApplication":
            self.chatapp = entity
            print "IRC: Found ChatApp!"
            self.scene.disconnect("EntityCreated(Scene::Entity*, AttributeChange::Type)", self.onNewEntity)

    def message(self, source, target, message):
        if target[0] == "#":
            s = "<%s:%s> %s" % (target, source[0], message)
        else:
            s = "-%s- %s" % (source, message)

        print "IRC:", s

        if self.chatapp is not None:
            self.chatapp.Exec(4, "ServerSendMessage", s)            

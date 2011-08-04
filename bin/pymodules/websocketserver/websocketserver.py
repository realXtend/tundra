import math
import random
import json
import socket #just for exception handling for restarts here

import circuits
import eventlet
from eventlet import websocket
from PythonQt.QtGui import QVector3D as Vec3
from PythonQt.QtGui import QQuaternion as Quat

import naali

import async_eventlet_wsgiserver

clients = set()
connections = dict()

class NaaliWebsocketServer(circuits.BaseComponent):
    instance = None
    def __init__(self, cfsection=None):
        circuits.BaseComponent.__init__(self)

        self.config = cfsection or dict()
        self.port = int(self.config.get('port', 9999))
        
        self.sock = eventlet.listen(('0.0.0.0', self.port))
        self.server = async_eventlet_wsgiserver.server(self.sock, handle_clients)
        print "websocket server started."

        naali.server.connect("UserConnected(int, UserConnection*)", self.onUserConnected)
        naali.server.connect("UserDisconnected(int, UserConnection*)", self.onUserDisconnected)

        NaaliWebsocketServer.instance = self
    
    def newclient(self, connectionid):
        id = self.scene.NextFreeId()
        naali.server.UserConnected(connectionid, 0)

        # Return the id of the connection
        return id

    def onUserConnected(self, connid, user):
        # If user is none we are probably dealing with websocket connection
        if user is None:
            self.note("New user connection from websocket: %d" % connid)
        else:
            self.note("New user connected: %s" % user.GetProperty("username"))

    def onUserDisconnected(self, connid, user):
        if user is None:
            self.note("Used disconnected from websocket: %d" % connid)
        else:
            self.note("User %s disconnected." % user.GetProperty("username"))

    def note(self, data):
        print 'ws', data

    def removeclient(self, connectionid):
        naali.server.UserDisconnected(connectionid, 0)

    @circuits.handler("on_sceneadded")
    def on_sceneadded(self, name):
        '''Connects to various signal when scene is added'''
        self.scene = naali.getScene(name)

        self.scene.connect("AttributeChanged(IComponent*, IAttribute*, AttributeChange::Type)", self.onAttributeChanged)

        self.scene.connect("EntityCreated(Scene::Entity*, AttributeChange::Type)", self.onNewEntity)

        self.scene.connect("ComponentAdded(Scene::Entity*, IComponent*, AttributeChange::Type)", self.onComponentAdded)

        self.scene.connect("EntityRemoved(Scene::Entity*, AttributeChange::Type)", self.onEntityRemoved)

    @circuits.handler("update")
    def update(self, t):
        if self.server is not None:
            self.server.next()

    @circuits.handler("on_exit")
    def on_exit(self):
        # Need to figure something out what to do and how
        pass

    def onAttributeChanged(self, component, attribute, changeType):
        #FIXME Only syncs hard coded ec_placeable
        #Maybe get attribute or something

        #FIXME Find a better way to get component name
        component_name = str(component).split()[0]

        #Let's only sync EC_Placeable
        if component_name != "EC_Placeable":
            return

        entity = component.GetParentEntity()

        # Don't sync local stuff
        if entity.IsLocal():
            return

        ent_id = entity.id

        data = component.GetAttributeQVariant('Transform')
        transform = list()

        transform.extend([data.position().x(), data.position().y(), data.position().z()])
        transform.extend([data.rotation().x(), data.rotation().y(), data.rotation().z()])
        transform.extend([data.scale().x(), data.scale().y(), data.scale().z()])

        sendAll(['setAttr', {'id': ent_id, 
                             'component': component_name,
                             'Transform': transform}])

    def onNewEntity(self, entity, changeType):
        sendAll(['addEntity', {'id': entity.id}])

        if entity.name == "ChatApplication":
            self.chatapp = entity
            print "WebSocket: Found ChatApp!"
            self.init_chatapp_handlers()


    def onComponentAdded(self, entity, component, changeType):
        #FIXME Find a better way to get component name
        component_name = str(component).split()[0]

        # Just sync EC_Placeable and EC_Avatar since they are currently the
        # only ones that are used in the client
        if component_name not in ["EC_Placeable", "EC_Avatar"]:
            return
        #FIXME
        if component_name == "EC_Avatar":
            sendAll(['addComponent', {'id': entity.id, 'component': "EC_Avatar"}])
        elif component_name == "EC_Mesh":
            sendAll(['addComponent', {'id': entity.id, 'component': "EC_Mesh", 'url': 'FIXME'}])
        else:
            data = component.GetAttributeQVariant('Transform')
            transform = list()

            transform.extend([data.position().x(), data.position().y(), data.position().z()])
            transform.extend([data.rotation().x(), data.rotation().y(), data.rotation().z()])
            transform.extend([data.scale().x(), data.scale().y(), data.scale().z()])

            sendAll(['addComponent', {'id': entity.id, 
                                 'component': component_name,
                                 'Transform': transform}])


    def onEntityRemoved(self, entity, changeType):
        sendAll(['removeEntity', {'id': entity.id}])

    def init_chatapp_handlers(self):
        csm = self.chatapp.Action("ClientSendMessage")
        csm.connect("Triggered(QString, QString, QString, QStringList)", self.onClientSendMessage)

    def onClientSendMessage(self, sender, msg):
        sendAll(['chatMessage', {'sender': sender, 'msg': msg}])


def sendAll(data):
    for client in clients:
        client.send(json.dumps(data))        

@websocket.WebSocketWSGI
def handle_clients(ws):
    clients.add(ws)
    
    # Don't do this! Figure out a way to fake a kNet connection or
    # something.
    connectionid = random.randint(1000, 10000)
    
    scene = NaaliWebsocketServer.instance.scene

    while True:
        # "main loop" for the server. When your done with the
        # connection break from the loop. It is important to remove
        # the socket from clients set

        try:
            msg = ws.wait()
        except socket.error:
            #if there is an error we simply quit by exiting the
            #handler. Eventlet should close the socket etc.
            break

        if msg is None:
            # if there is no message the client will Quit
            break

        try:
            function, params = json.loads(msg)
        except ValueError, error:
            print error
            continue

        if function == 'CONNECTED':
            ws.send(json.dumps(['initGraffa', {}]))

            myid = NaaliWebsocketServer.instance.newclient(connectionid)
            connections[myid] = connectionid
            ws.send(json.dumps(['setId', {'id': myid}]))
            
            #FIXME don't sync locals
            xml = scene.GetSceneXML(True)

            ws.send(json.dumps(['loadScene', {'xml': str(xml)}]))

        elif function == 'Action':
            action = params.get('action')
            args = params.get('params')
            id = params.get('id')
            av = scene.GetEntityByNameRaw("Avatar%s" % connections[id])

            av.Exec(1, action, args)

        elif function == 'chatMessage':
            sender = params.get('sender')
            message = params.get('msg')
            chatApp = scene.GetEntityByNameRaw("ChatApplication")
            if chatApp is None:
                continue
            chatApp.Exec(2, "ClientSendMessage", sender, message)
            
                
        elif function == 'reboot':
            break

    # Remove connection
    NaaliWebsocketServer.instance.removeclient(connectionid)
            
    clients.remove(ws)

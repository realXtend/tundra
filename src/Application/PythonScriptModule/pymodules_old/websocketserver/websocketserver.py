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
    def __init__(self):

        circuits.BaseComponent.__init__(self)
        self.sock = eventlet.listen(('0.0.0.0', 9999))
        self.server = async_eventlet_wsgiserver.server(self.sock, handle_clients)
        print "websocket server started."

        NaaliWebsocketServer.instance = self
    
    def newclient(self, connectionid):
        id = self.scene.NextFreeId()
        naali.server.UserConnected(connectionid, 0)

        # Return the id of the connection
        return id

    def removeclient(self, connectionid):
        naali.server.UserDisconnected(connectionid, 0)

    @circuits.handler("on_sceneadded")
    def on_sceneadded(self, name):
        '''Connects to various signal when scene is added'''
        self.scene = naali.getScene(name)

        self.scene.connect("AttributeChanged(IComponent*, IAttribute*, AttributeChange::Type)", onAttributeChanged)

        self.scene.connect("EntityCreated(Scene::Entity*, AttributeChange::Type)", onNewEntity)

        self.scene.connect("ComponentAdded(Scene::Entity*, IComponent*, AttributeChange::Type)", onComponentAdded)

        self.scene.connect("EntityRemoved(Scene::Entity*, AttributeChange::Type)", onEntityRemoved)

    @circuits.handler("update")
    def update(self, t):
        if self.server is not None:
            self.server.next()

    @circuits.handler("on_exit")
    def on_exit(self):
        # Need to figure something out what to do and how
        pass

def sendAll(data):
    for client in clients:
        client.send(json.dumps(data))        

def onAttributeChanged(component, attribute, changeType):
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

def onNewEntity(entity, changeType):
    sendAll(['addEntity', {'id': entity.id}])
    print entity

def onComponentAdded(entity, component, changeType):
    #FIXME Find a better way to get component name
    component_name = str(component).split()[0]

    # Just sync EC_Placeable and EC_Mesh since they are currently the
    # only ones that are used in the client
    if component_name not in ["EC_Placeable", "EC_Mesh"]:
        return

    if component_name == "EC_Mesh":

        sendAll(['addComponent', {'id': entity.id, 'component': component_name, 'url': 'ankka.dae'}])
    else:
        data = component.GetAttributeQVariant('Transform')
        transform = list()

        transform.extend([data.position().x(), data.position().y(), data.position().z()])
        transform.extend([data.rotation().x(), data.rotation().y(), data.rotation().z()])
        transform.extend([data.scale().x(), data.scale().y(), data.scale().z()])

        sendAll(['addComponent', {'id': entity.id, 
                             'component': component_name,
                             'Transform': transform}])

    print entity.id, component

def onEntityRemoved(entity, changeType):
    print "Removing", entity
    sendAll(['removeEntity', {'id': entity.id}])

@websocket.WebSocketWSGI
def handle_clients(ws):
    print 'START', ws
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

        print msg

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
                
        elif function == 'reboot':
            break

    # Remove connection
    NaaliWebsocketServer.instance.removeclient(connectionid)
            
    clients.remove(ws)
    print 'END', ws

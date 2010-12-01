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
import mathutils

import async_eventlet_wsgiserver

clients = set()

class NaaliWebsocketServer(circuits.BaseComponent):
    instance = None

    def __init__(self):
        circuits.BaseComponent.__init__(self)
        self.sock = eventlet.listen(('0.0.0.0', 9999))
        self.server = async_eventlet_wsgiserver.server(self.sock, handle_clients)
        print "websocket server started."

        NaaliWebsocketServer.instance = self
    
    def newclient(self, clientid):
        naali._naali.server.UserConnected(clientid, 0)
        
    @circuits.handler("on_sceneadded")
    def on_sceneadded(self, name):
        '''Connects to various signal when scene is added'''
        self.scene = naali.getScene(name)
        self.scene.connect("AttributeChanged(IComponent*, IAttribute*, AttributeChange::Type)", onAttributeChanged)

        self.scene.connect("EntityCreated(Scene::Entity*, AttributeChange::Type)",onNewEntity)

        self.scene.connect("ComponentAdded(Scene::Entity*, IComponent*, AttributeChange::Type)", onComponentAdded)

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
    #FIXME Find a better way to get component name
    component_name = str(component).split()[0]

    #We don't need no stinkin' rigid body
    if component_name == "EC_RigidBody":
        return

    ent_id = component.GetParentEntity().Id
    attribute = attribute

    sendAll(['setAttr', {'id': ent_id, 'component': component_name}])

def onNewEntity(entity, changeType):
    sendAll(['addEntity', {'id': entity.Id}])
    print entity

def onComponentAdded(entity, component, changeType):
    #FIXME Find a better way to get component name
    component_name = str(component).split()[0]

    #We don't need no stinkin' rigid body
    if component_name == "EC_RigidBody":
        return


    sendAll(['addComponent', {'id': entity.Id, 'component': component_name}])
    print entity.Id, component

@websocket.WebSocketWSGI
def handle_clients(ws):
    print 'START', ws
    myid = random.randrange(1,10000)
    clients.add(ws)
    
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
            # if there is no message the client has quit. 
            break

        try:
            function, params = json.loads(msg)
        except ValueError, error:
            print error
            continue

        if function == 'CONNECTED':
            ws.send(json.dumps(['initGraffa', {}]))

            NaaliWebsocketServer.instance.newclient(myid)

            ws.send(json.dumps(['setId', {'id': myid}]))

        elif function == 'Action':
            action = params.get('action')
            args = params.get('params')
            id = params.get('id')
            av = scene.GetEntityByNameRaw("Avatar%s" % id)

            av.Exec(myid, action, args)
                
        elif function == 'reboot':
            break

    clients.remove(ws)
    print 'END', ws

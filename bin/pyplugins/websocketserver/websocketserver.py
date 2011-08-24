import math
import random
import json
import socket #just for exception handling for restarts here

import sys
sys.path.append('/usr/lib/pymodules/python2.7/') #wtf
sys.path.append('/usr/lib/python2.7/dist-packages/')
import eventlet
from eventlet import websocket
from PythonQt.QtGui import QVector3D as Vec3
from PythonQt.QtGui import QQuaternion as Quat

import tundra

sys.path.append('./pyplugins/websocketserver') #omg
import websocketserver.async_eventlet_wsgiserver as async_eventlet_wsgiserver

clients = set()
connections = dict()

sock = eventlet.listen(('0.0.0.0', 9999))
print "websocket server started."

scene = None

def newclient(connectionid):
    if scene is not None:
        id = scene.NextFreeId()
        tundra.Server().UserConnected(connectionid, 0)

        # Return the id of the connection
        return id

    else:
        tundra.LogWarning("Websocket server got a client connection, but has no scene - what to do?")

def removeclient(connectionid):
    tundra.Server().UserDisconnected(connectionid, 0)

def on_sceneadded(name):
    '''Connects to various signal when scene is added'''
    global scene
    scene = tundra.Scene().GetDefaultSceneRaw() #name)
    print "Using scene:", scene.name, scene

    scene.connect("AttributeChanged(IComponent*, IAttribute*, AttributeChange::Type)", onAttributeChanged)
    scene.connect("EntityCreated(Scene::Entity*, AttributeChange::Type)", onNewEntity)

    scene.connect("ComponentAdded(Scene::Entity*, IComponent*, AttributeChange::Type)", onComponentAdded)

    scene.connect("EntityRemoved(Scene::Entity*, AttributeChange::Type)", onEntityRemoved)

    
def update(t):
    if server is not None:
        server.next()
        #print '.',

#def on_exit(self):
        # Need to figure something out what to do and how

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

            myid = newclient(connectionid)
            connections[myid] = connectionid
            ws.send(json.dumps(['setId', {'id': myid}]))
            
            #FIXME don't sync locals
            if scene is not None:
                xml = scene.GetSceneXML(True)
                ws.send(json.dumps(['loadScene', {'xml': str(xml)}]))
            else:
                tundra.LogWarning("Websocket Server: handling a client, but doesn't have scene :o")

        elif function == 'Action':
            action = params.get('action')
            args = params.get('params')
            id = params.get('id')

            if scene is not None:
                av = scene.GetEntityByNameRaw("Avatar%s" % connections[id])
                av.Exec(1, action, args)
            else:
                tundra.LogError("Websocket Server: received entity action, but doesn't have scene :o")
                
        elif function == 'reboot':
            break

    # Remove connection
    removeclient(connectionid)
            
    clients.remove(ws)
    print 'END', ws

server = async_eventlet_wsgiserver.server(sock, handle_clients)
tundra.Frame().connect("Updated(float)", update)

sceneapi = tundra.Scene()
sceneapi.SetDefaultScene("TundraServer")
sceneapi.connect("OnSceneAdded(QString)", on_sceneadded)
on_sceneadded("TundraServer")



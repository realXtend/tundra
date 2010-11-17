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

SPAWNPOS = 131.806, 48.9571, 28.7691 #at the end of pier in w.r.o:9000

class NaaliWebsocketServer(circuits.BaseComponent):
    instance = None

    def __init__(self):
        circuits.BaseComponent.__init__(self)
        self.sock = eventlet.listen(('0.0.0.0', 9999))
        self.server = async_eventlet_wsgiserver.server(self.sock, handle_clients)
        print "websocket server started."

        NaaliWebsocketServer.instance = self
        self.clientavs = {}

    def newclient(self, clientid, position, orientation):
        ent = naali.createMeshEntity("Jack.mesh")

        ent.placeable.Position = Vec3(position[0], position[1], position[2])

        ent.placeable.Orientation = Quat(mathutils.euler_to_quat(orientation))

        print "New entity for web socket presence at", ent.placeable.Position

        self.clientavs[clientid] = ent

    def updateclient(self, clientid, position, orientation):
        ent = self.clientavs[clientid]
        ent.placeable.Position = Vec3(position[0], position[1], position[2])
        ent.placeable.Orientation = Quat(mathutils.euler_to_quat(orientation))

    @circuits.handler("update")
    def update(self, t):
        if self.server is not None:
            self.server.next()

    @circuits.handler("on_exit")
    def on_exit(self):
        # Need to figure something out what to do and how
        pass

clients = set()
       
def sendAll(data):
    for client in clients:
        client.send(json.dumps(data))        

@websocket.WebSocketWSGI
def handle_clients(ws):
    print 'START', ws
    myid = random.randrange(1,10000)
    clients.add(ws)
    
    scene = naali.getScene("World")
    
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
            # if there is no message the client will quit. 
            break

        try:
            function, params = json.loads(msg)
        except ValueError, error:
            print error
            continue

        if function == 'CONNECTED':
            ws.send(json.dumps(['initGraffa', {}]))

            x, y, z = SPAWNPOS[0], SPAWNPOS[1], SPAWNPOS[2]
            start_position = (x, y, z)
            start_orientation = (1.57, 0, 0)
            NaaliWebsocketServer.instance.newclient(myid, start_position, start_orientation)

            ws.send(json.dumps(['setId', {'id': myid}]))
            sendAll(['addEntity', {'id': myid}])
            sendAll(['addComponent', {'id': myid, 'component': 'EC_Mesh', 'url': 'http://localhost:8000/WebNaali/ankka.dae'}])
            sendAll(['addComponent',{'id': myid, 'component': 'EC_Placeable',
                                     'x': x,
                                     'y': y,
                                     'z': z,
                                     'rotx': 1.57,
                                     'roty': 0,
                                     'rotz': 0}])

            # ents = scene.GetEntitiesWithComponentRaw("EC_DynamicComponent")

            # for ent in ents:
            #     id = ent.Id
            #     position = ent.placeable.Position.x(), ent.placeable.Position.y(), ent.placeable.Position.z()
            #     orientation = mathutils.quat_to_euler(ent.placeable.Orientation)
            #     sendAll(['addObject', {'id': id, 'position': position, 'orientation': orientation, 'xml': scene.GetEntityXml(ent).data()}])

        elif function == 'Naps':
            ws.send(json.dumps(['logMessage', {'message': 'Naps itelles!'}]))
            
        elif function == 'setAttr':
            id = params.get('id')
            component = params.get('component')
            
            if component == 'EC_Placeable':
                # What to do here?
                ent = NaaliWebsocketServer.instance.clientavs[myid]
                position = ent.placeable.Position
                orientation = mathutils.quat_to_euler(ent.placeable.Orientation)

                x = params.get('x', position.x())
                y = params.get('y', position.y())
                z = params.get('z', position.z())
                rotx = params.get('rotx', orientation[0])
                roty = params.get('roty', orientation[1])
                rotz = params.get('rotz', orientation[2])
                11
                NaaliWebsocketServer.instance.updateclient(myid, (x, y, z), (rotx, roty, rotz))

            ents = scene.GetEntitiesWithComponentRaw("EC_OpenSimPresence")
            for ent in ents:
                
                x = ent.placeable.Position.x()
                y = ent.placeable.Position.y()
                z = ent.placeable.Position.z()

                orientation = mathutils.quat_to_euler(ent.placeable.Orientation)

                id = ent.Id

                sendAll(['addEntity', {'id': id}])
                sendAll(['addComponent', {'id': id, 'component': 'EC_Mesh', 'url': 'http://localhost:8000/WebNaali/ankka.dae'}])
                sendAll(['addComponent', {'id': id, 'component': 'EC_Placeable'}])

                sendAll(['setAttr',
                         {'id': id,
                          'component': 'EC_Placeable',
                          'x': x,
                          'y': y,
                          'z': z,
                          'rotx': orientation[0],
                          'roty': orientation[1],
                          'rotz': orientation[2]}])

        elif function == 'updateObject':
            id = params['id']
            data = params['data']


            entity = scene.GetEntityRaw(id)
            component = entity.GetComponentRaw('EC_DynamicComponent', 'door')
            
            component.SetAttribute('opened', data['opened'])
                    
        elif function == 'setSize':
            y_max = params['height']
            x_max = params['width']

        elif function == 'reboot':
            break


    clients.remove(ws)
    print 'END', ws

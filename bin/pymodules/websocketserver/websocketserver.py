import math
import random
import json
import socket #just for exception handling for restarts here

import circuits
import eventlet
from eventlet import websocket

import naali
import rexviewer as r
from PytxhonQt.QtGui import QVector3D as Vec3


import async_eventlet_wsgiserver

SPAWNPOS = 131.806, 48.9571, 28.7691 #at the end of pier in w.r.o:9000

class NaaliWebsocketServer(circuits.BaseComponent):
    instance = None

    def __init__(self):
        circuits.BaseComponent.__init__(self)
        print "websocket listen.."
        self.sock = eventlet.listen(('0.0.0.0', 9999))
        print "..done"
        self.server = async_eventlet_wsgiserver.server(self.sock, hello_world)
        print "websocket server started."

        NaaliWebsocketServer.instance = self
        self.previd = 500000
        self.clientavs = {}

    def newclient(self, clientid, pos):
        #self.clients.add()
        ent = r.createEntity("avatar.mesh", self.previd)
        self.previd += 1

        ent.placeable.Position = Vec3(pos[0], pos[1], SPAWNPOS[2])
        print "New entity for web socket presence at", ent.placeable.Position

        self.clientavs[clientid] = ent

    def updateclient(self, clientid, pos):
        ent = self.clientavs[clientid]
        ent.placeable.Position = Vec3(pos[0], pos[1], SPAWNPOS[2])

    @circuits.handler("update")
    def update(self, t):
        #print "websocket server"
        #print "x"
        if self.server is not None:
            self.server.next()

    @circuits.handler("on_exit")
    def on_exit(self):
        for client in clients:
            client.socket.close()
        self.sock.close()
        self.server = None

clients = set()
       
def sendAll(data):
    for client in clients:
        client.send(json.dumps(data))        

@websocket.WebSocketWSGI
def hello_world(ws):
    print 'START', ws
    myid = random.randrange(1,10000)
    clients.add(ws)
    
    while True:
            
        try:
            msg = ws.wait()
        except socket.error:
            break
        print msg
        if msg is None:
            break
        try:
            function, params = json.loads(msg)
        except ValueError, error:
            print error

        if function == 'CONNECTED':
            ws.send(json.dumps(['initGraffa', {}]))
            #x = random.randrange(10, 180)
            #y = random.randrange(10, 180)
            x, y = SPAWNPOS[0], SPAWNPOS[1]
            NaaliWebsocketServer.instance.newclient(myid, (x, y))

            ws.send(json.dumps(['setId', {'id': myid}]))
            sendAll(['newAvatar', {'id': myid, 'x': x, 'y': y, 'dx' : 0, 'dy': 0, 'angle': 0, 'speed': 0}])

        elif function == 'Naps':
            ws.send(json.dumps(['logMessage', {'message': 'Naps itelles!'}]))
        elif function in ['up', 'down', 'left', 'right']:
            
            id = params.get('id')
            x = params.get('x')
            y = params.get('y')
            dx = params.get('dx')
            dy = params.get('dy')
            angle = params.get('angle')
            speed = params.get('speed')

            if function == 'left':
                angle -= math.pi/16
            elif function == 'right':
                angle += math.pi/16
            elif function == 'up':
                speed = max(-2, speed - 1)
              
            elif function == 'down':
                speed = min(2, speed + 1)

            sendAll(['updateAvatar',
                     {'id': id,
                      'angle': angle,
                      'x': x,
                      'y': y,
                      'dx': dx,
                      'dy': dy,
                      'speed': speed,
                      }])
            
        elif function == 'giev update':
            for id, data in params.items():

                x = data.get('x')
                y = data.get('y')
                dx = data.get('dx')
                dy = data.get('dy')
                angle = data.get('angle')
                speed = data.get('speed')

                if x < 3 or x >= x_max - 3:
                    speed *= -1

                if y < 3 or y > y_max - 3:
                    speed *= -1

                
                dx = -round(math.cos(-(angle - math.pi/2)) * speed, 3)
                dy = -round(math.sin(angle - math.pi/2) * speed, 3)

                x += dx
                y += dy
                NaaliWebsocketServer.instance.updateclient(myid, (x, y))

                s = naali.getScene("World")
                ids = s.GetEntityIdsWithComponent("EC_OpenSimPresence")
                ents = [r.getEntity(id) for id in ids]
                
                for ent in ents:
                    x = ent.placeable.Position.x()
                    y = ent.placeable.Position.y()
                    id = ent.id

                    sendAll(['updateAvatar',
                             {'id': id,
                              'angle': angle,
                              'x': x,
                              'y': y,
                              'dx': dx,
                              'dy': dy,
                              'speed': speed,
                              }])

                
        elif function == 'setSize':
            y_max = params['height']
            x_max = params['width']

        elif function == 'reboot':
            break
    clients.remove(ws)
    print 'END', ws

def handle_move():
    pass

if __name__ == '__main__':
    sock = eventlet.listen(('0.0.0.0', 9999))
    server = async_eventlet_wsgiserver.server(sock, hello_world)
    
    while True:
        server.next()
        print 'TICK'

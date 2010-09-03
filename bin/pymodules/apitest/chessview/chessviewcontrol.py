import os
import time
import array #the full board is an int array in pychess

#import process_chess_runner
#import multiprocess_chess_runner
#import subprocess
from subprocess import PIPE
import async_subprocess as asub

import gettext
gettext.install("pychess", unicode=1)
from pychess.Utils.const import * #for EMPTY and such in board making

from PythonQt.QtGui import QVector3D as Vec3
import circuits
import rexviewer as r
    
class SubprocessCommunicator(circuits.Process):
    def run(self):
        subproc = asub.Popen(["python", "pymodules/apitest/chessview/pychessview.py"], stdout=PIPE)
        print "chess runner started from Naali SubprocessCommunicator", os.getpid()
        time.sleep(0.1) #a little time for the process to start
    
        while 1:
            time.sleep(0.1) #10fps ought to be well enough for chess moves!
            dead = subproc.poll()
            if dead is not None:
                #print "ChessViewControl: asubproc died"
                print "_",
                continue
            rec = asub.recv_some(subproc)
            #rec = subproc.communicate()
            #print "REC:", rec
            if rec:
                self.push(circuits.Event(rec), "rec")


class ChessViewControl(circuits.BaseComponent):
    def __init__(self):
        circuits.BaseComponent.__init__(self)
        #p = process_chess_runner.PychessRunner()
        #p = multiprocess_chess_runner.PychessRunner()
        #p.start()
        #self.subproc = asub.Popen(["python", "pymodules/apitest/chessview/pychessview.py"], stdout=PIPE)
        #print "chess runner started from Naali main process", os.getpid()
        self.subcomms = SubprocessCommunicator(self)
        self.subcomms.start()

        self.board = None
        self.entidcount = 120000

        self.cmdhandlers = {
            'BEGIN': self.begin,
            'MOVE': self.move,
            'REMOVE': self.remove
            }
    
    @circuits.handler("rec")
    def rec(self, data):
        #print "ChessViewControl rec data:", data
        msgs = data.split('\n')
        for msg in msgs:
            try:
                cmd, params = msg.split(':')
            except ValueError:
                print "invalid message, expects command:params - ", msg
            else:
                self.cmdhandlers[cmd](params)

    def begin(self, boarddata):
        self.board = array.array('B')
        self.board.fromstring(boarddata)
        print "got board:", self.board

        for cor, piece in enumerate(self.board):
            if piece != EMPTY:
                x = cor / 8
                y = cor % 8
                self.addpiece(x, y, piece)

    def addpiece(self, x, y, piece):
        ent = r.createEntity("axes.mesh", self.entidcount)
        self.entidcount += 1 #make the api func use next available id XXX
        wx = 120 + x
        wy = 120 + y
        wz = 25
        ent.placeable.Position = Vec3(wx, wy, wz)
        ent.placeable.Scale = Vec3(0.1, 0.1, 0.1)

    def move(self, movedata):
        print "MOVE:", movedata

    def remove(self, removedata):
        print "REMOVE:", removedata

    @circuits.handler("update")
    def update(self, t):
        pass
        #print "x"
        #print self.subproc.communicate()
        #s = self.subproc
        #dead = s.poll()
        #if dead is not None:
            #print "ChessViewControl: asubproc died"
        #    return
    
        #rec = asub.recv_some(s)
        #print "REC:", rec

def main():
    v = ChessViewControl()
    v.start()
    while 1:
        pass

"""in parsemove.py now
def removePiece(tcord, tpiece, opcolor):
    print "REMOVE:", tcord, tpiece, opcolor

def addPiece(tcord, piece, color):
    print "ADD:", tcord, piece, opcolor

def move(fcord, tcord, piece, color):
    print "REMOVE:", tcord, tpiece, opcolor
"""

if __name__ == '__main__':
    main()

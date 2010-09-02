import os
import time

#import process_chess_runner
#import multiprocess_chess_runner
#import subprocess
from subprocess import PIPE
import async_subprocess as asub

import circuits
    
class SubprocessCommunicator(circuits.Process):
    def run(self):
        subproc = asub.Popen(["python", "pymodules/apitest/chessview/pychessview.py"], stdout=PIPE)
        print "chess runner started from Naali SubprocessCommunicator", os.getpid()
        time.sleep(1) #a little time for the process to start
    
        while 1:
            rec = asub.recv_some(subproc)
            #rec = subproc.communicate()
            #print "REC:", rec
            if rec:
                self.push(circuits.Event(rec), "rec")
            time.sleep(0.1) #10fps ought to be well enough for chess moves!

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
    
    @circuits.handler("rec")
    def rec(self, msg):
        print "ChessViewControl got msg:", msg
        commands = msg.split('\n')
        for c in commands:
            print c

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
    #v.start()
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

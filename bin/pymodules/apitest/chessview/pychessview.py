import sys
from sys import stdin, stdout

import time

import gtk
import __builtin__, gettext, gtk.glade
gettext.install("pychess", unicode=1)
t = gettext.translation("pychess", fallback=True)
__builtin__.__dict__["ngettext"] = t.ungettext #evilness copy-paste from pychess startup script

import pychess.Main
from pychess.System.Log import log

import pychess.widgets.ionest as i #current hack to get access to GameModel
import parsemove #the own thing here for parsing move data from pychess
import msg #talking to the parent process, called directly from parsemove too

def initchess():
    p = pychess.Main.PyChess(None)
    log.log("Started from Naali\n")
    #print "PyChess in Naali"

g = None
prev_board = None
prev_arBoard = None

def updatechess():
    #print ".",
    global g, prev_board, prev_arBoard

    for _ in range(100):
        gtk.main_iteration(block=False)
    #gtk.gdk.threads_enter()

    if g is None and i.globalgamemodel is not None:
        g = i.globalgamemodel
        board = g.boards[-1]
        msg.send("BEGIN:%s" % board.board.arBoard.tostring())

    if g is not None:
        #print g.players, id(g.boards[-1]), g.boards[-1]
        board = g.boards[-1]
        if board is not prev_board:
            #print "BOARD:", board
            if len(board.board.history) > 0:
                prevmove = board.board.history[-1][0]
                parsemove.parsemove(board, prev_arBoard, prevmove)

            prev_board = board
            prev_arBoard = board.board.arBoard[:]

def main():
    initchess()
    gtk.gdk.threads_init()
    while 1:
        updatechess()
        time.sleep(0.001)

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

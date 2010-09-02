import gtk
import __builtin__, gettext, gtk.glade
gettext.install("pychess", unicode=1)
t = gettext.translation("pychess", fallback=True)
__builtin__.__dict__["ngettext"] = t.ungettext #evilness copy-paste from pychess startup script

import pychess.Main
from pychess.System.Log import log

import pychess.widgets.ionest as i #current hack to get access to GameModel
import parsemove #the own thing here for parsing move data from pychess

try:
    import circuits
except:
    print "circuits not there, can still run as standalone outside Naali"
else:
    class ChessView(circuits.BaseComponent):
        def __init__(self):
            initchess()

        @circuits.handler("update")
        def update(self, t):
            updatechess()

def initchess():
    p = pychess.Main.PyChess(None)
    gtk.gdk.threads_init()

    log.log("Started from Naali\n")
    print "PyChess in Naali"

g = None
prev_board = None
prev_arBoard = None

def updatechess():
    global g, prev_board, prev_arBoard

    gtk.main_iteration(block=False)

    if g is None and i.globalgamemodel is not None:
        g = i.globalgamemodel

    if g is not None:
        #print g.players, id(g.boards[-1]), g.boards[-1]
        board = g.boards[-1]

        if board is not prev_board:
            print "BOARD:", board
            if len(board.board.history) > 0:
                prevmove = board.board.history[-1][0]
                parsemove.parsemove(board, prev_arBoard, prevmove)

            prev_board = board
            prev_arBoard = board.board.arBoard[:]

def main():
    initchess()
    while 1:
        #print ".",
        updatechess()

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

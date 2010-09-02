import __builtin__, gettext, gtk.glade
gettext.install("pychess", unicode=1)
t = gettext.translation("pychess", fallback=True)
__builtin__.__dict__["ngettext"] = t.ungettext #evilness copy-paste from pychess startup script

from pychess.System.Log import log
import parsemove

import pychess.Main
p = pychess.Main.PyChess(None)

import gtk
gtk.gdk.threads_init()

log.log("Started\n")
print "Hello"

#gtk.main()
import pychess.widgets.ionest as i
g = None
prev_board = None
prev_arBoard = None

while 1:
    #print ".",
    gtk.main_iteration(block=False)
    
    if g is None and i.globalgamemodel is not None:
        g = i.globalgamemodel

    if g is not None:
        #print g.players, id(g.boards[-1]), g.boards[-1]
        board = g.boards[-1]

        if board is not prev_board:
            #log.log("BOARD: %s" % str(board))
            #log.log("ENPASS %s" % str(b.enpassant))
            #log.log("HISTORY: %s" % b.history)
            print "BOARD:", board
            if len(board.board.history) > 0:
                prevmove = board.board.history[-1][0]
                parsemove.parsemove(board, prev_arBoard, prevmove)

            prev_board = board
            prev_arBoard = board.board.arBoard[:]

"""in parsemove.py now
def removePiece(tcord, tpiece, opcolor):
    log.log("REMOVE")
    print "REMOVE:", tcord, tpiece, opcolor

def addPiece(tcord, piece, color):
    log.log("ADD")
    print "ADD:", tcord, tpiece, opcolor

def move(fcord, tcord, piece, color):
    log.log("MOVE")
    print "REMOVE:", tcord, tpiece, opcolor
"""

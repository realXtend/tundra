import circuits
import os

class PychessRunner(circuits.Process):
    g = None
    i = None
    prev_board = None
    prev_arBoard = None

    def init_in_process(self):
        import gtk
        import __builtin__, gettext, gtk.glade
        gettext.install("pychess", unicode=1)
        t = gettext.translation("pychess", fallback=True)
        __builtin__.__dict__["ngettext"] = t.ungettext #evilness copy-paste from pychess startup script

        import pychess.Main
        from pychess.System.Log import log

        import pychess.widgets.ionest as i #current hack to get access to GameModel
        self.i = i

        p = pychess.Main.PyChess(None)
        print "PyChess in Naali PychessRunner, PID", os.getpid()

        gtk.gdk.threads_init()

    def updatechess(self):
        import gtk
        import parsemove #the own thing here for parsing move data from pychess

        #print ".",
        gtk.main_iteration(block=False)

        if self.g is None and self.i.globalgamemodel is not None:
            self.g = self.i.globalgamemodel

        if self.g is not None:
            #print g.players, id(g.boards[-1]), g.boards[-1]
            board = self.g.boards[-1]

            if board is not self.prev_board:
                print "BOARD:", board
                if len(board.board.history) > 0:
                    prevmove = board.board.history[-1][0]
                    parsemove.parsemove(board, self.prev_arBoard, prevmove) #should use prev_board for right current color XXX

                self.prev_board = board
                self.prev_arBoard = board.board.arBoard[:]

    def run(self):
        self.init_in_process()
        while 1:
            self.updatechess()

#if __name__ == '__main__':
#    main()
#    def run(self):
        #initchess()
        #gtk.gdk.threads_init()
#        while 1:
            #gtk.main_iteration(block=False)
#            print "-",

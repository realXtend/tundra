"""a non-blocking, non-threaded non-multiprocessing circuits web server"""

try:
    import circuits
except ImportError: #not running within the viewer, but testing outside it
    import sys
    sys.path.append('..')

from circuits.web import Controller #Server

class WebController(Controller):
    #now loaded as a special case in circuits_manager itself
    #~ def __init__(self):
        #~ Controller.__init__(self)
        #~ self.rungen = None #(Server(8000) + self).run_once() #added a yield to the core
        
    #~ def update(self, deltatime):
        #~ if self.rungen is not None:
            #~ self.rungen.next()
            #~ print "*",
        
    def index(self):
        return "Hello World!"


#~ if __name__ == '__main__':
    #~ while 1:
        #~ print ".",

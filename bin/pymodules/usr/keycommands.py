"""a module that executes test commands bound to keys,
to help py api dev and testing"""

try:
    import rexviewer as r
except ImportError: #not running under rex
    import mockviewer as r
from circuits import Component

class KeyCommander(Component):
    OIS_KEY_C = 46 #it might be possible to use pyois from pyogre to get these
    OIS_KEY_UP = 200
    OIS_KEY_PERIOD = 52
    EVENTHANDLED = True
    
    def __init__(self):
        Component.__init__(self)
        
        """
        This dictionary has the keyboard commands, well the
        event ids for different keys that are currently enabled
        on the python side.
        """
        self.keymap = {
            self.OIS_KEY_PERIOD: self.run_commandpy
        }
        
        """
        This has the events and overrides different ones... explain better! ;)
        """
        self.inputmap = {
            #r.MoveForwardPressed: self.overrideForwardWalking #overrides the moveforward event
        }
        
    def on_keydown(self, key, mods):
        #print "on_keydown call -> ", key
        if key in self.keymap:
            r.eventhandled = self.EVENTHANDLED
            self.keymap[key]()
        else:
            r.eventhandled = False

    def on_keyup(self, key, mods):
        pass
        
    def on_input(self, evid):
        #print "Commander got input", evid
        if evid in self.inputmap:
            r.eventhandled = self.EVENTHANDLED
            self.inputmap[evid]()
        else:
            r.eventhandled = False
    
    #uncomment this for raycasting tests
    #~ def on_mousemove(self, mouseinfo):
        #~ print "MouseMove", mouseinfo.x, mouseinfo.y
        #~ ent = r.rayCast(mouseinfo.x, mouseinfo.y)
        #~ print "Got entity:", ent
        #~ if ent is not None:
            #~ print "Entity position is", ent.pos
        
    def run_commandpy(self):
        #print "Command:"
        r.eventhandled = self.EVENTHANDLED
        import command
        command = reload(command)
        
    def overrideForwardWalking(self):
        print "MoveForward called, STOP, it's Hammer time!"
        r.eventhandled = self.EVENTHANDLED
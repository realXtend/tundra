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
        self.inputmap = {
            #r.MoveForwardPressed: self.run_commandpy,
            #r.KeyPressed: self.run_commandpy, 
            self.OIS_KEY_PERIOD: self.run_commandpy
            #self.OIS_KEY_UP: self.overrideForwardWalking
        }
        
    def on_keydown(self, key, mods):#, retfunc):
        #print "on_keydown call -> ", key
        #retfunc(self.FORWARDEVENTS)
        if key in self.inputmap:
            self.inputmap[key]()

    def on_keyup(self, key, mods):
        pass
        
    def on_input(self, evid):
        #print "Commander got input", evid
        
        if evid in self.inputmap:
            self.inputmap[evid]()
        
    def run_commandpy(self):
        #print "Command:"
        r.eventhandled = self.EVENTHANDLED
        import command
        command = reload(command)
        
    def overrideForwardWalking(self):
        #print "Arrow UP pressed, don't move?"
        r.eventhandled = self.EVENTHANDLED
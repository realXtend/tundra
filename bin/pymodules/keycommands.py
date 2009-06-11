"""a module that executes test commands bound to keys,
to help py api dev and testing"""

try:
    import rexviewer as r
except ImportError: #not running under rex
    import mockviewer as r
from circuits import Component

class KeyCommander(Component):
    OIS_KEY_C = 46
    def __init__(self):
        Component.__init__(self)
        self.inputmap = {
            #r.MoveForwardPressed: self.run_commandpy,
            #r.KeyPressed: self.run_commandpy, 
            self.OIS_KEY_C: self.run_commandpy
        }
    
    def on_keydown(self, key):
        print "on_keydown call:", key
        kc = key[0]
        if kc in self.inputmap:
            self.inputmap[kc]()
            
    def on_input(self, evid):
        #print "Commander got input", evid

        if evid in self.inputmap:
            self.inputmap[evid]()
            
    def run_commandpy(self):
        #print "Command:"
        import command
        command = reload(command)
        
    
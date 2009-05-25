try:
    import rexviewer as r
except ImportError: #not running under rex
    import mockviewer as r

from circuits import Component

inputtests = {
    r.MoveForwardPressed: "^",
    r.MoveForwardReleased: "."
}

class TestModule(Component):
    """Was a wish of how a py written module might like to have the system.
    Is now ported to circuits, so is made a Circuits.Component.
    
    This could for example be the CommunicationsModule, right?
    Or something that implements as a custom control type..
    Here a generic test thing."""
    
    def __init__(self):
        Component.__init__(self)
        self.data = 1
        
    def update(self, deltatime):
        if self.data == 1:
            self.data = 0
        else:
            self.data = 1
        #print self.data, deltatime
        
    def on_chat(self, frm=None, msg=None):
        print "Test Module received chat message:", frm, msg
        
    def on_input(self, evid):
        print "Test Module received input event:", evid
        
        print inputtests[evid]        

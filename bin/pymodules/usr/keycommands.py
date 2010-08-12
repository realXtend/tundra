"""a module that executes test commands bound to keys,
to help py api dev and testing"""

try:
    import rexviewer as r
except ImportError: #not running under rex
    import mockviewer as r
from circuits import Component
from naali import inputcontext

from PythonQt.QtCore import Qt

class KeyCommander(Component):
    EVENTHANDLED = True
    
    def __init__(self):
        Component.__init__(self)
        """
        This dictionary has the keyboard commands, well the
        event ids for different keys that are currently enabled
        on the python side. Now using qt input constants.
        
        (KEY_ID, MOD_ID): methodToCall()
        """

        self.inputmap = {
            (Qt.Key_Period, Qt.AltModifier): self.run_commandpy,
            (Qt.Key_F11, Qt.NoModifier): self.restart_modulemanager
            }
            #r.MoveForwardPressed: self.overrideForwardWalking #overrides the moveforward event

        inputcontext.disconnect('KeyPressed(KeyEvent*)', self.on_keypressed)
        inputcontext.connect('KeyPressed(KeyEvent*)', self.on_keypressed)
        #XXX a temp hack to have restart work in login screen / ether too
        uiview = r.getUiView()
        uiview.connect("PythonRestartRequest()", self.restart_modulemanager)
        
    # handle KeyPressed event from naali.inputcontext
    def on_keypressed(self, key):
        keyb = (key.keyCode, key.modifiers)
        if keyb in self.inputmap:
            return self.inputmap[keyb]()
        else:
            return False
    
    #uncomment this for raycasting tests
    #~ def on_mousemove(self, mouseinfo):
        #~ print "MouseMove", mouseinfo.x, mouseinfo.y
        #~ ent = r.rayCast(mouseinfo.x, mouseinfo.y)
        #~ print "Got entity:", ent
        #~ if ent is not None:
            #~ print "Entity position is", ent.pos
    
    #~ def on_entityupdated(self, id):
        #~ e = r.getEntity(id)
        #~ print "Entity got touched", id#, "got", e, e.pos
    
    #~ def on_login(self, *args):
        #~ print "THIS WOOORKS!!!"

    def run_commandpy(self):
        execfile("pymodules/usr/command.py")
        return True
        
    def restart_modulemanager(self):
        import modulemanager
        modulemanager.ModuleManager.instance.restart()
        return False
        
    def overrideForwardWalking(self):
        print "MoveForward called, STOP, it's Hammer Time!"
        return True
        
    def toggle_editgui(self):
        try:
            if not r.c.canvas.IsHidden():
                r.c.canvas.Hide()
            else:
                r.c.canvas.Show()
            return True
        except:
            print "toggle editgui failed, r.c is missing most liekly."

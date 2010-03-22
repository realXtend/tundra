"""a module that executes test commands bound to keys,
to help py api dev and testing"""

try:
    import rexviewer as r
except ImportError: #not running under rex
    import mockviewer as r
from circuits import Component

OIS_KEY_C = 46 #it might be possible to use pyois from pyogre to get these
OIS_KEY_UP = 200
OIS_KEY_PERIOD = 52
OIS_KEY_BACKSPACE = 14
OIS_KEY_ALT = 256
OIS_KEY_M = 50

class KeyCommander(Component):
    EVENTHANDLED = True
    
    def __init__(self):
        Component.__init__(self)
        """
        This dictionary has the keyboard commands, well the
        event ids for different keys that are currently enabled
        on the python side.
        
        (KEY_ID, MOD_ID): methodToCall()
        """
        self.keymap = {
            (OIS_KEY_PERIOD, OIS_KEY_ALT): self.run_commandpy,
            #(OIS_KEY_M, OIS_KEY_ALT): self.toggle_editgui,
            (OIS_KEY_BACKSPACE, OIS_KEY_ALT): self.restart_modulemanager
        }
        
        """
        These are the input events as they have already been translated within Naali internals.
        For example both 'w' and 'uparrow' keypresses cause a MoveForwardPressed input event.
        """
        self.inputmap = {
            r.PyRestart: self.restart_modulemanager,
            r.PyRunCommand: self.run_commandpy
            #r.MoveForwardPressed: self.overrideForwardWalking #overrides the moveforward event
        }

        #XXX a temp hack to have restart work in login screen / ether too
        uiview = r.getUiView()
        uiview.connect("PythonRestartRequest()", self.restart_modulemanager)
        
    def on_keydown(self, key, mods, callback):
        #print "on_keydown call -> ", key, mods, (key, mods), self.keymap.has_key((key, mods))
        if (key, mods) in self.keymap:
            eventhandled = self.keymap[(key, mods)]()
            callback(eventhandled)
        else:
            callback(False)

    def on_keyup(self, key, mods, callback):
        pass
        
    def on_input(self, evid, callback):
        #print "Commander got input", evid
        if evid in self.inputmap:
            eventhandled = self.inputmap[evid]()
            callback(eventhandled)
        else:
            callback(False)
    
    #uncomment this for raycasting tests
    #~ def on_mousemove(self, mouseinfo, callback):
        #~ print "MouseMove", mouseinfo.x, mouseinfo.y
        #~ ent = r.rayCast(mouseinfo.x, mouseinfo.y)
        #~ print "Got entity:", ent
        #~ if ent is not None:
            #~ print "Entity position is", ent.pos
    
    #~ def on_entityupdated(self, id, callback):
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

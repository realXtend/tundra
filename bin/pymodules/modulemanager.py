import rexviewer as r

class ModuleManager:
    #was not calling update to not confuse with the eventhandlers
    def run(self, elapsedtime):
        pass
        #print ".",
        #print elapsedtime
        
    """here was using 'on_chat' but changed to viewer event names,
    perhaps clearest as these are what the viewer calls,
    core authors know exactly which is which here."""
    def RexNetMsgChatFromSimulator(self, frm, message):
        pass
        #print "Manager got on_chat", frm, message
        
    def INPUT_EVENT(self, evid):
        pass
        #print "Manager got input event:", evid
    def KEY_INPUT_EVENT(self, evid, keycode, keymod):
        pass
        #print "Manager got key_input event:", evid, keycode, keymod
    def MOUSE_MOVEMENT(self, x_abs, y_abs, x_rel, y_rel):
        pass
        #print "Manager got mouse movement", x_abs, y_abs, x_rel, y_rel
    def MOUSE_CLICK(self, mb_click, x_abs, y_abs, x_rel, y_rel):
        pass
        #print "Manager got mouse click", mb_click, x_abs, y_abs, x_rel, y_rel
    def ENTITY_UPDATED(self, id):
        pass
        #print "Manager got an entity updated", id
    def LOGIN_INFO(self, *args):
        pass
        #print "Login updated", args
    
    def exit(self):
        print "exiting module manager"
        pass

USE_CIRCUITS = True

if USE_CIRCUITS:
    try:
        import core.circuits_manager
    except ImportError:
        #print "circuits_manager not available"
        r.logInfo("Circuits Manager not available.")
    else:
        #print "Using circuits_manager"
        r.logInfo("Using Circuits Manager.")
        ModuleManager = core.circuits_manager.ComponentRunner
        
if __name__ == '__main__': #running from cmdline, testing without viewer
    """this is that the viewer does"""
    m = ModuleManager()
    import time
    try:
        while 1:
                m.run(0.1)
                time.sleep(0.1)
        #except KeyboardInterrupt:
    finally:
        print "stopping circuitsmanager"
        m.exit()
            
        
            
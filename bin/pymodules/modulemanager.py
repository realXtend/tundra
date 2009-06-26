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
        print "Manager got on_chat", frm, message
        
    def INPUT_EVENT(self, evid):
        print "Manager got input event:", evid
    def KEY_INPUT_EVENT(self, evid, keycode, keymod):
        print "Manager got key_input event:", evid, keycode, keymod
    def MOUSE_INPUT(self, x_abs, y_abs, x_rel, y_rel):
        print "Manager got mouse input", x_abs, y_abs, x_rel, y_rel
                
    def exit(self):
        print "exiting module manager"
        pass

USE_CIRCUITS = True

if USE_CIRCUITS:
    try:
        import core.circuits_manager
    except ImportError:
        print "circuits_manager not available"
    else:
        print "Using circuits_manager"
        ModuleManager = core.circuits_manager.ComponentRunner
        
if __name__ == '__main__': #running from cmdline, testing without viewer
    """this is that the viewer does"""
    m = ModuleManager()
    import time
    while 1:
        m.run(0.1)
        time.sleep(0.1)
class ModuleManager:
    #was not calling update to not confuse with the eventhandlers
    def run(self, elapsedtime):
        pass
        #print ".",
        #print elapsedtime
        
    """here was using 'on_chat' but changed to viewer event names,
    perhaps clearest as these are what the viewer calls,
    core authors know exactly which is which here."""
    def RexNetMsgChatFromSimulator(self, m): #frm, message):
        pass
        print "Manager got on_chat", m #frm, message
        
    def exit(self):
        print "exiting module manager"
        pass
        
try:
    import eventsource
except ImportError:
    print "eventsource not available"
else:
    print "eventsource hook loaded "

USE_PYGLET = False #have misbehaviour now, at least upon restarts,
USE_CIRCUITS = True

if USE_CIRCUITS:
    try:
        import circuits_manager
    except ImportError:
        print "circuits_manager not available"
    else:
        print "Using circuits_manager"
        ModuleManager = circuits_manager.ComponentRunner

#probably 'cause it uses a hidden window and doesn't play nice in the embedded context
if USE_PYGLET:
    try:
        import pyglet_manager
    except ImportError:
        print "pyglet_manager not available"
        import sys
        print sys.path
        pass #pyglet_manager not available, that's fine
        #other things are errors inside it, and those i wanna see.
    else:
        print "Using PygletManager"
        ModuleManager = pyglet_manager.PygletManager
        
if __name__ == '__main__': #running from cmdline, testing without viewer
    """this is that the viewer does"""
    m = ModuleManager()
    import time
    while 1:
        m.run(0.1)
        time.sleep(0.1)
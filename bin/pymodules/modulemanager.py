class ModuleManager:
    #was not calling update to not confuse with the eventhandlers
    def run(self): #elapsedtime
        pass
        #print ".",
        
    """here was using 'on_chat' but changed to viewer event names,
    perhaps clearest as these are what the viewer calls,
    core authors know exactly which is which here."""
    def RexNetMsgChatFromSimulator(self, m): #frm, message):
        print "Manager got on_chat", m #frm, message

try:
    import pyglet_manager
except ImportError:
    pass #pyglet_manager not available, that's fine
    #other things are errors inside it, and those i wanna see.
else:
    ModuleManager = pyglet_manager.PygletManager

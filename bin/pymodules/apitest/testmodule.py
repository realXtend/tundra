class TestModule:
    """Was wish of how a py written module might like to have the system.
    Was earlier made to work with pyglet, that is in pyglet_testmodule.py
    The current implementation is the TestModule in circuits_testmodule.py

    This could for example be the CommunicationsModule, right?
    Or something that implements as a custom control type..
    Here a generic test thing."""
    
    def __init__(self):
        self.data = 1
        
    #@viewer.event
    def not_update(self, deltatime):
        if self.data == 1:
            self.data = 0
        else:
            self.data = 1
        print self.data,
            
"""
why not allow plain functions as event handlers too,
the python module of it, i.e. the file, can be considered as the module,
so no classes are required.
"""

"""XXX NOTE: the first notation a) somehow breaks c) if done after it,
post to pyglet / submit a bug about it?"""
# a)
@manager.event
def update(deltatime):
    pass #e.g. move an object w.r.t to something (like time?)
    print "_",

# b)
def arbitary_func(deltatime):
    print "|",
manager.push_handlers(update=arbitary_func)
manager.push_handlers(on_chat=arbitary_func)

# c)
tm = TestModule()
manager.push_handlers(update=tm.not_update)

@manager.event
def on_chat(frm, msg):
    print "Chat from %s: %s" % (frm, msg)
    
"""
update may be nice as a method, but the real thing are the events.
"""

#~ @e.communication.PRESENCE_STATUS_UPDATE
#~ def sparkle_presence_updates():
    #~ """a custom visual thing: some local sparkles upon IM presence updates.
    #~ what data do we have in these events? the im participant id?
    #~ is that associated to an avatar anywhere? so could e.g. get the location..
    #~ """
    #~ create_sparkle((0,2, 0,2), 0.5) #in upper right corner, for half a sec

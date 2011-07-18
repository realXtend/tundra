"""drafting and designing a manager for python written modules.
the current plan is that there'll be a Manager that the PythonScriptModule
instanciates, and which then loads the python modules that are to be autoloaded,
and handles their event registrations, passing incoming events to them etc.
that will be prototyped here in pure py, and perhaps reused in the actual
impl if it seems that a py written manager makes sense within the c++ framework too.

currently here is only test code for modules and handelers that would
use the manager, nothing of the Manager itself yet.
"""

from circuits import handler, Event, Component, Manager

# Create a new Manager
m = Manager()

class Update(Event):
    pass #to have the deltatime as event data
    
class Chat(Event):
    pass

class ComponentRunner(Component):
    def run(self, deltatime):
        print ".",
        m.push(Update(), "update")
        m.push(Chat("hip", "hop"), "on_chat")
    
runner = ComponentRunner()

class TestModule(Component):
    """A wish of how a py written module might like to have the system.
    This could for example be the CommunicationsModule, right?
    Or something that implements as a custom control type..
    Now attempting a generic test thing."""
    
    def __init__(self):
        Component.__init__(self)
        self.data = 1
        
    def update(self):
        if self.data == 1:
            self.data = 0
        else:
            self.data = 1
        print self.data,
        
    def on_chat(self, frm=None, msg=None):
        print "Test Module received chat message:", frm, msg
        
tm = TestModule()
m += tm # Equivalent to: tm.register(m)

"""
why not allow plain functions as event handlers too,
the python module of it, i.e. the file, can be considered as the module,
so no classes are required.

circuits has it so that you are supposed to make components.
"""

#~ @e.communication.PRESENCE_STATUS_UPDATE
#~ def sparkle_presence_updates():
    #~ """a custom visual thing: some local sparkles upon IM presence updates.
    #~ what data do we have in these events? the im participant id?
    #~ is that associated to an avatar anywhere? so could e.g. get the location..
    #~ """
    #~ create_sparkle((0,2, 0,2), 0.5) #in upper right corner, for half a sec
    
if __name__ == '__main__':
    import time
    m.start()
    while True:
        runner.run(0.1)
        time.sleep(0.1)



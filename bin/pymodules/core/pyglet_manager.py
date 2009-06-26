"""drafting and designing a manager for python written modules.
the current plan is that there'll be a Manager that the PythonScriptModule
instanciates, and which then loads the python modules that are to be autoloaded,
and handles their event registrations, passing incoming events to them etc.
that will be prototyped here in pure py, and perhaps reused in the actual
impl if it seems that a py written manager makes sense within the c++ framework too.

currently here is only test code for modules and handelers that would
use the manager, nothing of the Manager itself yet.
"""

#e = rexviewer.event

import pyglet
import eventsource #a hack to expose the manager instance from here to modules

import pollable_pyglet_app
pollable_pyglet_app.enable() #replaces the event loop impl
pump = pyglet.app.event_loop.pump()

class PygletManager(pyglet.event.EventDispatcher):
    def __init__(self):
        print "PygletManager init"
        self.register_event_type('update')
        self.register_event_type('on_chat')
        
        eventsource.viewer = self
                
    def run(self, deltatime=0.1):
        self.dispatch_event('update', deltatime)
        pump.next()
        #print ".",        
    
    def RexNetMsgChatFromSimulator(self, message): #frm, message):
        frm = "Bob"
        #print "Manager got on_chat", m #frm, message
        self.dispatch_event('on_chat', frm, message)
        
    def exit(self):
        pyglet.app.event_loop.exit()
        pump.next() #XXX whould this take several calls? my understanding is that not.

 
"""the viewer ScriptService likes to instanciate now,
might be good to refactor so that get the instance there?"""
#manager = PygletManager() 

#pyglet doesn't have managers so no confusion here, 
#circuits has though so was calling 'runner' or something
        
if __name__ == '__main__':    
    pyglet.clock.schedule_interval(manager.run, 0.1)
    #pyglet.app.run()
    """don't want to run a blocking thing here now,
    as want to use the api without threads first,
    so added a crude simple pump using yield to pyglet.app"""
    while 1:
        #print "*",
        pump.next()

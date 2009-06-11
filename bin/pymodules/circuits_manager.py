"""drafting and designing a manager for python written modules.
the current plan is that there'll be a Manager that the PythonScriptModule
instanciates, and which then loads the python modules that are to be autoloaded,
and handles their event registrations, passing incoming events to them etc.
that will be prototyped here in pure py, and perhaps reused in the actual
impl if it seems that a py written manager makes sense within the c++ framework too.

currently here is only test code for modules and handelers that would
use the manager, nothing of the Manager itself yet.
"""
try:
    import rexviewer as r
except ImportError: #not running under rex
    import mockviewer as r
from circuits import handler, Event, Component, Manager

#is not identical to the c++ side, where x and y have abs and rel
#XXX consider making identical and possible wrapping of the c++ type
from collections import namedtuple
MouseInfo = namedtuple('MouseInfo', 'x y rel_x rel_y')

class Key(Event): pass
class Update(Event): pass     
class Chat(Event): pass    
class Input(Event): pass
class MouseMove(Event): pass
    
class ComponentRunner(Component):
    instance = None

    def __init__(self):
        # instanciated from the c++ side, as modulemanager there
        assert self.instance is None
        ComponentRunner.instance = self #is used as a singleton now

        # Create a new circuits Manager
        self.m = Manager()

        Component.__init__(self)
        
        m = self.m
        
        #or __all__ in pymodules __init__ ? (i.e. import pymodules would do that)
        import autoload
        autoload.load(m)
        
        #m.start()

    def run(self, deltatime=0.1):
        #XXX should this be using the __tick__ mechanism of circuits, and how?
        #print ".",
        self.m.send(Update(deltatime), "update")
        
    def RexNetMsgChatFromSimulator(self, frm, message):
        self.m.send(Chat(frm, message), "on_chat")
        
    def INPUT_EVENT(self, evid):
        """Note: the PygameDriver for circuits has a different design:
        there the event data is Key, and the channel either "keydown" or "keyup",
        and mouse and clicks are different events and channels.
        Here we have no way to differentiate presses/releases,
        'cause the c++ internals don't do that apart from the constant name.
        """
        self.m.send(Input(evid), "on_input")
        #print "circuits_manager ComponentRunner got input event:", evid       
        
    def KEY_INPUT_EVENT(self, evid, keycode, keymod):
        """Handles key inputs, creates a Circuits Key event with the data provided
        WIP, since on_keydown call doesn't work for now, resorted in using Input(keycode)
        instead, works similarly but still not the way it should
        """
        
        #print "CircuitManager received KEY_INPUT (event:", evid, "key:", keycode, "mods:", keymod, ")",
        #key = Key(keycode, keymod)
        
        if evid == r.KeyPressed:
            #self.m.send(key, "on_keydown")
            self.m.send(Input(keycode), "on_input") #well, since on_keydown doesnt work for some odd reason, using this instead for now
            #print "pressed."
        
        #~ elif evid == r.KeyReleased:
            #~ #self.m.send(key, "on_keyup")
            #~ print "released."
        #~ else:
            #~ print "Nothing found."
            #~ return
            
    def MOUSE_INPUT(self, x_abs, y_abs, x_rel, y_rel):
        i = MouseInfo(x_abs, y_abs, x_rel, y_rel)
        #print "Manager got mouse input", i
        self.m.send(MouseMove(i), "on_mousemove")
        
    def exit(self):
        print "Circuits manager stopping."
        self.m.stop() #is this needed?
        
#TestModule moved to own file (err, module)

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
    runner = ComponentRunner()

    import time
    while True:
        runner.run(0.1)
        runner.RexNetMsgChatFromSimulator("main..", "hello")
        time.sleep(0.1)



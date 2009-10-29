"""Circuits using implementation of a Python module (plugin) manager for Naali.

here is a Manager that the PythonScriptModule instanciates, 
and which then loads the python modules that are to be autoloaded.
This handles their event registrations, passing incoming events to them etc.
"""
try:
    import rexviewer as r
except ImportError: #not running under rex
    import mockviewer as r
from circuits import handler, Event, Component, Manager, Debugger

#is not identical to the c++ side, where x and y have abs and rel
#XXX consider making identical and possible wrapping of the c++ type
#from collections import namedtuple
#MouseInfo = namedtuple('MouseInfo', 'x y rel_x rel_y')
from core.mouseinfo import MouseInfo

class Key(Event): pass
class Update(Event): pass     
class Chat(Event): pass    
class Input(Event): pass
class MouseMove(Event): pass
class MouseClick(Event): pass
class EntityUpdate(Event): pass
class Exit(Event): pass
class LoginInfo(Event): pass
    
class ComponentRunner(Component):
    instance = None
    
    def __init__(self):
        # instanciated from the c++ side, as modulemanager there
        #assert self.instance is None
        Component.__init__(self)
        ComponentRunner.instance = self #is used as a singleton now
        
        self.mouseinfo = MouseInfo(0,0,0,0)
        #m.start() #instead we tick() & flush() in update
        
        self.firstrun = True
        self.eventhandled = False
        self.start()
        
    def start(self):
        # Create a new circuits Manager
        #ignevents = [Update, MouseMove]
        ignchannames = ['update', 'on_mousemove', 'on_keydown', 'on_input', 'on_mouseclick', 'on_entityupdated', 'on_exit', 'on_keyup', 'on_login']
        ignchannels = [('*', n) for n in ignchannames]
        self.m = Manager() + Debugger(IgnoreChannels = ignchannels) #IgnoreEvents = ignored)
        
        #or __all__ in pymodules __init__ ? (i.e. import pymodules would do that)
        if self.firstrun:
            import autoload
            self.firstrun = False
        else: #reload may cause something strange sometimes, so better not do it always in production use, is just a dev tool.
            #print "reloading autoload"
            import autoload
            autoload = reload(autoload)            
        #print "Autoload module:", autoload
        autoload.load(self.m)
                    
    def run(self, deltatime=0.1):
        #print ".",
        m = self.m
        m.send(Update(deltatime), "update") #XXX should this be using the __tick__ mechanism of circuits, and how?
        m.tick()
        m.flush()
        #XXX NOTE: now that we tick & flush, circuits works normally, and we could change from send() to push() for the events
        
    def RexNetMsgChatFromSimulator(self, frm, message):
        self.m.send(Chat(frm, message), "on_chat")
        
    def callback(self, value):
        self.eventhandled = value
        
    def INPUT_EVENT(self, evid):
        """Note: the PygameDriver for circuits has a different design:
        there the event data is Key, and the channel either "keydown" or "keyup",
        and mouse and clicks are different events and channels.
        Here we have no way to differentiate presses/releases,
        'cause the c++ internals don't do that apart from the constant name.
        """
        #print "circuits_manager ComponentRunner got input event:", evid       
        self.eventhandled = False
        self.m.send(Input(evid, self.callback), "on_input")
        return self.eventhandled
        
    def KEY_INPUT_EVENT(self, evid, keycode, keymod):
        """Handles key inputs, creates a Circuits Key event with the data provided
        WIP, since on_keydown call doesn't work for now, resorted in using Input(keycode)
        instead, works similarly but still not the way it should
        """
        
        #print "CircuitManager received KEY_INPUT (event:", evid, "key:", keycode, "mods:", keymod, ")",
        self.eventhandled = False
        if evid == r.KeyPressed:
            self.m.send(Key(keycode, keymod, self.callback), "on_keydown")
        elif evid == r.KeyReleased:
            self.m.send(Key(keycode, keymod, self.callback), "on_keyup")
        return self.eventhandled
            
    def MOUSE_MOVEMENT(self, x_abs, y_abs, x_rel, y_rel):
        self.eventhandled = False
        self.mouseinfo.setInfo(x_abs, y_abs, x_rel, y_rel)
        #print "CircuitsManager got mouse input", self.mouseinfo, self.mouseinfo.x, self.mouseinfo.y
        self.m.send(MouseMove(self.mouseinfo, self.callback), "on_mousemove")
        return self.eventhandled
    
    def MOUSE_CLICK(self, mb_click, x_abs, y_abs, x_rel, y_rel):
        #print "CircuitsManager got mouse click", mb_click 
        self.eventhandled = False
        self.mouseinfo.setInfo(x_abs, y_abs, x_rel, y_rel)
        self.m.send(MouseClick(mb_click, self.mouseinfo, self.callback), "on_mouseclick")
        return self.eventhandled
        
    def ENTITY_UPDATED(self, id):
        #print "Entity updated!", id
        self.eventhandled = False
        self.m.send(EntityUpdate(id, self.callback), "on_entityupdated")
        return self.eventhandled

    def LOGIN_INFO(self, *args): 
        #print "Login Info", args
        self.eventhandled = False
        #self.m.send(LoginInfo(self.callback), "on_login")
        return self.eventhandled
        
    def exit(self):
        r.logInfo("Circuits manager stopping...")
        self.m.send(Exit(), "on_exit") #am not running the manager properly so the stop doesn't propagate to components. fix when switch to dev branch of circuits XXX
        self.m.stop() #was not going to components now so made the exit event above as a quick fix
        #XXX now that we are using flush() and tick(), does stop() propagate to components too?
        
    def restart(self):
        r.logInfo("Restarting python module manager, reloading plugin codes")
        self.exit()
        self.start()
        r.logInfo("...done python restart.")
                
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
        #runner.RexNetMsgChatFromSimulator("main..", "hello")
        """
        rvalue = runner.KEY_INPUT_EVENT(3, 46, 0)
        if rvalue:
            print "returned true"
        else:
            print "returned false"
            
        """
        runner.MOUSE_INPUT(5, 6, 7, 8)
        time.sleep(0.01)



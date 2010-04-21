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
from core.logger import NaaliLogger
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
class InboundNetwork(Event): pass
class GenericMessage(Event): pass
class Logout(Event): pass
    
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
        r.restart = False
        self.start()
        r.manager = self
        
    def start(self):
        # Create a new circuits Manager
        #ignevents = [Update, MouseMove]
        ignchannames = ['update', 'on_mousemove', 'on_mousedrag', 'on_keydown', 'on_input', 'on_mouseclick', 'on_entityupdated', 'on_exit', 'on_keyup', 'on_login', 'on_inboundnetwork', 'on_genericmessage', 'on_scene', 'on_entity_visuals_modified', 'on_logout']
        ignchannels = [('*', n) for n in ignchannames]
        
        # Note: instantiating Manager with debugger causes severe lag when running as a true windowed app (no console), so instantiate without debugger
        # Fix attempt: give the circuits Debugger a logger which uses Naali logging system, instead of the default which writes to sys.stderr
        # Todo: if the default stdout is hidden, the py stdout should be changed
        # to something that shows e.g. in console, so prints from scripts show
        # (people commonly use those for debugging so they should show somewhere)
        d = Debugger(IgnoreChannels = ignchannels, logger=NaaliLogger()) #IgnoreEvents = ignored)
        self.m = Manager() + d
        #self.m = Manager()

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
        
    """
    XXX this can be removed with a newer circuits that returns
    a Value object for triggered events, from where a return value
    or whether an error occurred can be seen:
    00:28 <@prologic> Value objects are used here
00:29 <@prologic> x will be an instance of a Value
00:29 <@prologic> returned immediately
00:29 <+antont> what happens with them on errors?
00:29 <@prologic> but only set when it'a associated event handlers are done and 
                  their associated values are done, etc, etc, etc
00:29 <@prologic> it can nest infinitely
00:29 <@prologic> if errors occur then x.errors == True
00:29 <@prologic> and x.value will contain a 3-item tuple of (etype, evalue, 
                  etraceback)
    """
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
            
    def MOUSE_DRAG_INPUT_EVENT(self, event, x_abs, y_abs, x_rel, y_rel):
        self.eventhandled = False
        self.mouseinfo.setInfo(x_abs, y_abs, x_rel, y_rel)
        #print "CircuitsManager got mouse movement", self.mouseinfo, self.mouseinfo.x, self.mouseinfo.y
        self.m.send(MouseMove(event, self.mouseinfo, self.callback), "on_mousedrag")
        return self.eventhandled
        
    def MOUSE_INPUT_EVENT(self, event, x_abs, y_abs, x_rel, y_rel):
        #print "CircuitsManager got a mouse click", mb_click, x_abs, y_abs, x_rel, y_rel
        self.eventhandled = False
        self.mouseinfo.setInfo(x_abs, y_abs, x_rel, y_rel)
        if event == r.MouseMove:
            self.m.send(MouseMove(event, self.mouseinfo, self.callback), "on_mousemove")
        else:
            self.m.send(MouseClick(event, self.mouseinfo, self.callback), "on_mouseclick")
        return self.eventhandled
        
    def SCENE_EVENT(self, evid, entid):
        self.m.send(EntityUpdate(evid, entid), "on_scene")
        
    def ENTITY_UPDATED(self, entid):
        #print "Entity updated!", entid
        self.eventhandled = False
        self.m.send(EntityUpdate(entid, self.callback), "on_entityupdated")
        return self.eventhandled

    def ENTITY_VISUALS_MODIFIED(self, entid):
        self.m.send(EntityUpdate(entid), "on_entity_visuals_modified")
        return False

    def LOGIN_INFO(self, *args): 
        #print "Login Info", args
        self.eventhandled = False
        #self.m.send(LoginInfo(self.callback), "on_login")
        return self.eventhandled
    
    def SERVER_DISCONNECTED(self, id):
        #print "Circuits got the server disconnection event."
        self.m.send(Logout(id), "on_logout")
        return False
        
    def INBOUND_NETWORK(self, id, name):
        #print "Circuits got network_in event:", id, name
        self.eventhandled = False
        self.m.send(InboundNetwork(id, name, self.callback), "on_inboundnetwork")
        return self.eventhandled
        ##r.randomTest(id) #for testing whether the id gotten is the same after a circulation on the python, note: worked

    def GENERIC_MESSAGE(self, typename, data):
        self.eventhandled = False
        #print "Circuits got Generic Message event:", data
        self.m.send(GenericMessage(typename, data), "on_genericmessage")
        return self.eventhandled
               
    def exit(self):
        r.logInfo("Circuits manager stopping...")
        self.m.send(Exit(), "on_exit") #am not running the manager properly so the stop doesn't propagate to components. fix when switch to dev branch of circuits XXX
        self.m.stop() #was not going to components now so made the exit event above as a quick fix
        #XXX now that we are using flush() and tick(), does stop() propagate to components too?
        
    def restart(self):
        #r.restart = True
        r.logInfo("Restarting python module manager, reloading plugin codes")
        self.exit()
        self.start()
        r.logInfo("...done python restart.")
        #r.restart = False
                
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



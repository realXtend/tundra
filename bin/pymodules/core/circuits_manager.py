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
class SceneAdded(Event): pass
class EntityUpdate(Event): pass
class Exit(Event): pass
class LoginInfo(Event): pass
class InboundNetwork(Event): pass
class GenericMessage(Event): pass
class Logout(Event): pass
class WorldStreamReady(Event): pass
    
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
        r.restart = False
        self.start()
        r.manager = self
        
    def start(self):
        # Create a new circuits Manager
        #ignevents = [Update, MouseMove]
        ignchannames = ['update', 'on_mousemove', 'on_mousedrag', 'on_keydown', 'on_input', 'on_mouseclick', 'on_entityupdated', 'on_exit', 'on_keyup', 'on_login', 'on_inboundnetwork', 'on_genericmessage', 'on_scene', 'on_entity_visuals_modified', 'on_logout', 'on_worldstreamready']
        ignchannels = [('*', n) for n in ignchannames]
        
        # Note: instantiating Manager with debugger causes severe lag when running as a true windowed app (no console), so instantiate without debugger
        # Fix attempt: give the circuits Debugger a logger which uses Naali logging system, instead of the default which writes to sys.stderr
        # Todo: if the default stdout is hidden, the py stdout should be changed
        # to something that shows e.g. in console, so prints from scripts show
        # (people commonly use those for debugging so they should show somewhere)
        #d = Debugger(IgnoreChannels = ignchannels, logger=NaaliLogger()) #IgnoreEvents = ignored)
        self.m = Manager() #+ d
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
        self.send_event(Update(deltatime), "update") #so that all components are updated immediately once for this frame
        #XXX should this be using the __tick__ mechanism of circuits, and how?
        m = self.m
        m.tick()
        while m: m.flush()
        #XXX NOTE: now that we tick & flush, circuits works normally, and we could change from send() to push() for the events

    def send_event(self, event, channel):
        """simulate sync sending of events using the async lib.
        needed to be able to return the info of whether the event is to be
        send to more handlers on the c++ side in the Naali event system"""
        m = self.m
        ret = m.push(event, channel)
        while m: m.flush() #circuits components evaluate to false when have no pending events left
        if not ret.errors and ret.value is not None:
            #print "EVENT:", event, ret.value
            return True in ret.value
        else:
            #did the debugger already show the traceback?
            return False
        
    def RexNetMsgChatFromSimulator(self, frm, message):
        self.send_event(Chat(frm, message), "on_chat")
        
    def INPUT_EVENT(self, evid):
        """Note: the PygameDriver for circuits has a different design:
        there the event data is Key, and the channel either "keydown" or "keyup",
        and mouse and clicks are different events and channels.
        Here we have no way to differentiate presses/releases,
        'cause the c++ internals don't do that apart from the constant name.
        """
        #print "circuits_manager ComponentRunner got input event:", evid       
        return self.send_event(Input(evid), "on_input")
        
    def KEY_INPUT_EVENT(self, evid, keycode, keymod):
        """Handles key inputs, creates a Circuits Key event with the data provided
        WIP, since on_keydown call doesn't work for now, resorted in using Input(keycode)
        instead, works similarly but still not the way it should
        """
        
        #print "CircuitManager received KEY_INPUT (event:", evid, "key:", keycode, "mods:", keymod, ")"
        #print r.KeyPressed, r.KeyReleased
        if evid == 39: #r.KeyPressed:
            return self.send_event(Key(keycode, keymod), "on_keydown")
        elif evid == 40: #r.KeyReleased:
            return self.send_event(Key(keycode, keymod), "on_keyup")
            
    def MOUSE_DRAG_INPUT_EVENT(self, event, x_abs, y_abs, x_rel, y_rel):
        self.mouseinfo.setInfo(x_abs, y_abs, x_rel, y_rel)
        #print "CircuitsManager got mouse movement", self.mouseinfo, self.mouseinfo.x, self.mouseinfo.y
        return self.send_event(MouseMove(event, self.mouseinfo), "on_mousedrag")
        
    def MOUSE_INPUT_EVENT(self, event, x_abs, y_abs, x_rel, y_rel):
        #print "CircuitsManager got a mouse click", mb_click, x_abs, y_abs, x_rel, y_rel
        #print "CircuitsManager", event
        self.mouseinfo.setInfo(x_abs, y_abs, x_rel, y_rel)
        if event == 60: #r.MouseMove:
            return self.send_event(MouseMove(event, self.mouseinfo), "on_mousemove")
            #return self.send_event(Mouse(event, self XXX
        else:
            return self.send_event(MouseClick(event, self.mouseinfo), "on_mouseclick")

##    def SCENE_EVENT(self, evid, entid):
##        return self.send_event(EntityUpdate(evid, entid), "on_scene")

    def SCENE_ADDED(self, name):
        return self.send_event(SceneAdded(name), "on_sceneadded")
        
    def ENTITY_UPDATED(self, entid):
        #print "Entity updated!", entid
        return self.send_event(EntityUpdate(entid), "on_entityupdated")

    def ENTITY_VISUALS_MODIFIED(self, entid):
        return self.send_event(EntityUpdate(entid), "on_entity_visuals_modified")

    def LOGIN_INFO(self, *args): 
        #print "Login Info", args
        #self.send_event(LoginInfo(), "on_login") #XXX so wasn't needed or?
        return False
    
    def SERVER_DISCONNECTED(self, id):
        #print "Circuits got the server disconnection event."
        return self.send_event(Logout(id), "on_logout")
        
    def INBOUND_NETWORK(self, id, name):
        #print "Circuits got network_in event:", id, name
        return self.send_event(InboundNetwork(id, name), "on_inboundnetwork")
        ##r.randomTest(id) #for testing whether the id gotten is the same after a circulation on the python, note: worked

    def GENERIC_MESSAGE(self, typename, data):
        #print "Circuits got Generic Message event:", data
        return self.send_event(GenericMessage(typename, data), "on_genericmessage")

    def WORLD_STREAM_READY(self, event_id):
        return self.send_event(WorldStreamReady(event_id), "on_worldstreamready")
               
    def exit(self):
        r.logInfo("Circuits manager stopping...")
        self.send_event(Exit(), "on_exit") #was originally not running the manager properly so the stop doesn't propagate to components. fix when switch to dev branch of circuits XXX
        self.m.stop() #was not going to components now so made the exit event above as a quick fix
        while self.m: self.m.flush()
        
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



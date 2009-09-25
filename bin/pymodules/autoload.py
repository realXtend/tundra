"""the modules that are to be loaded when the viewer starts.

add your module *class* (the circuits Component)
to the list called 'modules'. you need to import your pymodule
to have access here to the class, of course, and that can do
whatever loading that you want (in your module i.e. .py file).
"""

#reload-on-the fly test - how to make generic for all modules?
try:
    apitest.circuits_testmodule
except: #first run
    import apitest.circuits_testmodule
else:
    print "reloading apitest.circuits_testmodule"
    apitest.circuits_testmodule = reload(apitest.circuits_testmodule)
    
import usr.chathandler
import usr.keycommands
#import usr.mousecontrol
import editgui

try:
    import webserver.webcontroller
except ImportError: #socket not avaible in debugmode
    import circuits
    WebServer = circuits.Component #a dummy for debugmode
else:
    from webserver.webcontroller import WebServer
    
#import headtrack.control
import usr.sleeper

modules = [
    #apitest.circuits_testmodule.TestModule,
    usr.chathandler.ChatHandler,
    usr.keycommands.KeyCommander,
    usr.sleeper.Sleeper,
    #editgui.EditGUI,
    #WebServer
    #usr.mousecontrol.MouseControl,
]

#modules.append(headtrack.control.HeadTrack)

def load(circuitsmanager):
    for klass in modules:
        #~ modinst = klass()
        #~ circuitsmanager += modinst
        #print klass
        try:
            modinst = klass()            
        except Exception, exc:
            print "failed to instansciate pymodule", klass
            print exc
        else:
            circuitsmanager += modinst # Equivalent to: tm.register(m)
 
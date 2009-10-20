"""the modules that are to be loaded when the viewer starts.

add your module *class* (the circuits Component)
to the list called 'modules'. you need to import your pymodule
to have access here to the class, of course, and that can do
whatever loading that you want (in your module i.e. .py file).
"""
import rexviewer as r

#reload-on-the fly test - how to make generic for all modules?
try:
    apitest.circuits_testmodule
except: #first run
    import apitest.circuits_testmodule
else:
    r.logInfo("   reloading apitest.circuits_testmodule")
    apitest.circuits_testmodule = reload(apitest.circuits_testmodule)

try:
    usr.chathandler
except: #first run
    import usr.chathandler
else:
    r.logInfo("   reloading usr.chathandler")
    usr.chathandler = reload(usr.chathandler)

try:
    usr.keycommands
except: #first run
    import usr.keycommands
else:
    r.logInfo("   reloading usr.keycommands")
    usr.keycommands = reload(usr.keycommands)

try:
    editgui
except: #first run
    r.arrows = {}
    import editgui
else:
    r.logInfo("   reloading editgui")
    editgui = reload(editgui)

#~ try:
    #~ usr.mousecontrol
#~ except: #first run
    #~ import usr.mousecontrol
#~ else:
    #~ r.logInfo("   reloading usr.mousecontrol")
    #~ usr.mousecontrol = reload(usr.mousecontrol)

try:
    usr.sleeper
except: #first run
    import usr.sleeper
else:
    r.logInfo("   reloading usr.sleeper")
    usr.sleeper = reload(usr.sleeper)

#~ try:
    #~ headtrack.control
#~ except: #first run
    #~ import headtrack.control
#~ else:
    #~ r.logInfo("   reloading headtrack.control")
    #~ headtrack.control = reload(headtrack.control)

try:
    import webserver.webcontroller
except ImportError: #socket not avaible in debugmode
    import circuits
    WebServer = circuits.Component #a dummy for debugmode
else:
    from webserver.webcontroller import WebServer
    
modules = [
    #apitest.circuits_testmodule.TestModule,
    #usr.chathandler.ChatHandler,
    usr.keycommands.KeyCommander,
    #usr.sleeper.Sleeper,
    editgui.EditGUI,
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
 
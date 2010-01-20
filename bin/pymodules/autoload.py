"""the modules that are to be loaded when the viewer starts.

add your module *class* (the circuits Component)
to the list called 'modules'. you need to import your pymodule
to have access here to the class, of course, and that can do
whatever loading that you want (in your module i.e. .py file).
"""
import rexviewer as r
import sys

def load_module(modulename):
    print "Loading", modulename,
    try:
        exec(modulename)
    except NameError, e: 
        print "- initial import."
        exec("import %s" % modulename)
    else:
        print "- reload."
        exec("%s = reload(%s)" % (modulename, modulename))
    return sys.modules[modulename]

#reload-on-the fly test - how to make generic for all modules?
try:
    apitest.circuits_testmodule
except: #first run
    import apitest.circuits_testmodule
else:
    r.logDebug("   reloading apitest.circuits_testmodule")
    apitest.circuits_testmodule = reload(apitest.circuits_testmodule)

try:
    usr.chathandler
except: #first run
    import usr.chathandler
else:
    r.logDebug("   reloading usr.chathandler")
    usr.chathandler = reload(usr.chathandler)

try:
    usr.keycommands
except: #first run
    import usr.keycommands
else:
    r.logDebug("   reloading usr.keycommands")
    usr.keycommands = reload(usr.keycommands)

#~ usr.keycommands = load_module("usr.keycommands")

try:
    editgui.editgui #only_layout
except: #first run
    try:
        import editgui.editgui #only_layout
    except ImportError, e:
        print "couldn't load edigui:", e
else:
    r.logDebug("   reloading editgui")
    editgui.editgui = reload(editgui.editgui) #only_layout)

#~ editgui = load_module("editgui")

try:
    apitest.pythonqt_gui
except: #first run
    import apitest.pythonqt_gui
else:
    r.logDebug("   reloading apitest.pythonqt_gui")
    apitest.pythonqt_gui = reload(apitest.pythonqt_gui)

try:
    usr.sleeper
except: #first run
    import usr.sleeper
else:
    r.logDebug("   reloading usr.sleeper")
    usr.sleeper = reload(usr.sleeper)

#~ try:
    #~ usr.mousecontrol
#~ except: #first run
    #~ import usr.mousecontrol
#~ else:
    #~ r.logDebug("   reloading usr.mousecontrol")
    #~ usr.mousecontrol = reload(usr.mousecontrol)
    
#try:
#    headtrack.control
#except: #first run
#    import headtrack.control
#else:
#    r.logDebug("   reloading headtrack.control")
#    headtrack.control = reload(headtrack.control)
#import headtrack.control

try:
    import webserver.webcontroller
except ImportError: #socket not avaible in debugmode
    print "NOTE: not enabling webserver 'cause socket module not available"
    import circuits
    WebServer = circuits.Component #a dummy for debugmode
else:
    from webserver.webcontroller import WebServer

import apitest.testrunner
import mediaurlhandler.mediaurlhandler
    
modules = [
    #apitest.circuits_testmodule.TestModule,
    #usr.chathandler.ChatHandler,
    usr.keycommands.KeyCommander,
    #usr.sleeper.Sleeper,
    editgui.editgui.EditGUI, #only_layout.OnlyLayout,
    #mediaurlhandler.mediaurlhandler.MediaURLHandler,
    apitest.pythonqt_gui.TestGui,
    #WebServer,
    #usr.mousecontrol.MouseControl,
    #apitest.testrunner.TestLoginLogoutExit,
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

    #del modules #attempt to improve reloading, not keep refs to old versions
 

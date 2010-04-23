"""the modules that are to be loaded when the viewer starts.

add your module *class* (the circuits Component)
to the list called 'modules'. you need to import your pymodule
to have access here to the class, of course, and that can do
whatever loading that you want (in your module i.e. .py file).
"""
import rexviewer as r
import sys
import traceback

"""
this should be completed and put to use for all loading to get rid of the copy-paste here.
best example now is objectedit loading, 'cause it has the import exc handling too.
 
def freshimport(modulename): #aka. load_or_reload
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
"""

modules = []

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
    objectedit.objectedit
except: #first run
    try:
        import objectedit.objectedit
    except:  #ImportError, e:
        print "couldn't load objectedit:"
        traceback.print_exc()
    else:
        modules.append(objectedit.objectedit.ObjectEdit) #copy-pasted from the other successful case below :/
else:
    r.logDebug("   reloading objectedit")
    try:
        objectedit.objectedit = reload(objectedit.objectedit)
    except: #e.g. a syntax error in the source, reload fail
        print "couldn't reload objectedit:"
        traceback.print_exc()
    else:
        modules.append(objectedit.objectedit.ObjectEdit)

#~ editgui = load_module("editgui")

try:
    apitest.pythonqt_gui
except: #first run
    import apitest.pythonqt_gui
else:
    r.logDebug("   reloading apitest.pythonqt_gui")
    apitest.pythonqt_gui = reload(apitest.pythonqt_gui)

#~ try:
    #~ usr.anonlogin
#~ except: #first run
    #~ import usr.anonlogin
#~ else:
    #~ r.logDebug("   reloading usr.anonlogin")
    #~ usr.anonlogin = reload(usr.anonlogin)

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

try:
    loadurlhandler
except: #first run
    import loadurlhandler
else:
    loadurlhandler = reload(loadurlhandler)

import apitest.testrunner
import mediaurlhandler.mediaurlhandler
    
modules.extend([
    #apitest.circuits_testmodule.TestModule,
    #usr.chathandler.ChatHandler,
    usr.keycommands.KeyCommander,
    #usr.sleeper.Sleeper,
    #objectedit.objectedit.ObjectEdit, #now added to list at import due to new eror reporting tech
    #usr.anonlogin.AnonLogin,
    #mediaurlhandler.mediaurlhandler.MediaURLHandler,
    loadurlhandler.LoadURLHandler,
    #apitest.pythonqt_gui.TestGui,
    #WebServer,
    #usr.mousecontrol.MouseControl,
    #apitest.testrunner.TestLoginLogoutExit,
])

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
            traceback.print_exc()
        else:
            circuitsmanager += modinst # Equivalent to: tm.register(m)

    #del modules #attempt to improve reloading, not keep refs to old versions
 

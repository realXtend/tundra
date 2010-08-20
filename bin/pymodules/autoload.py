"""the modules that are to be loaded when the viewer starts.

add your module *class* (the circuits Component)
to the list called 'modules'. you need to import your pymodule
to have access here to the class, of course, and that can do
whatever loading that you want (in your module i.e. .py file).
"""
import rexviewer as r
import sys
import traceback

from core.freshimport import freshimport

#TODO: add reading these from a .ini file or something to ease custom modding
modulenames = [
    #("usr.sleeper", "Sleeper"),
    #("usr.camcontrol", "CameraController"),
    #("apitest.circuits_testmodule", "TestModule"),
    #("apitest.pythonqt_gui", "TestGui"),
    #("apitest.thread_test", "TestThread"),
    #("usr.chathandler", ChatHandler),
    ("usr.keycommands", "KeyCommander"),
    ("loadurlhandler", "LoadURLHandler"),
    ("mediaurlhandler.mediaurlhandler", "MediaURLHandler"),
    #("apitest.testrunner", "TestLoginLogoutExit",
    #("webserver.webcontroller", "WebServer"),
    ("localscene.localscene", "LocalScene"),
    ("objectedit.objectedit", "ObjectEdit"),
    ("estatemanagement.estatemanagement", "EstateManagement"),
    #("apitest.attrsync", "AnimationSync"),
    #("apitest.door", "DoorHandler"),
    #("apitest.jscomponent", "JavascriptHandler")
    ]

modules = []
for modname, compname in modulenames:
    m = freshimport(modname)
    if m is not None: #loaded succesfully. if not, freshload logged traceback
        c = getattr(m, compname)
        modules.append(c)

#modules.append(headtrack.control.HeadTrack)

def load(circuitsmanager):
    for klass in modules:
        #~ modinst = klass()
        #~ circuitsmanager += modinst
        #print klass
        try:
            modinst = klass()            
        except Exception, exc:
            r.logInfo("failed to instansciate pymodule %s" % klass)
            r.logInfo(traceback.format_exc())
        else:
            circuitsmanager += modinst # Equivalent to: tm.register(m)

    #del modules #attempt to improve reloading, not keep refs to old versions

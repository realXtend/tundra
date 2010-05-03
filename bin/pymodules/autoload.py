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

#import or reload, to support refreshing the code on the fly
sleeper = freshimport("usr.sleeper")
circuits_testmodule = freshimport("apitest.circuits_testmodule")
chathandler = freshimport("usr.chathandler")
keycommands = freshimport("usr.keycommands")
loadurlhandler = freshimport("loadurlhandler")

import apitest.testrunner
#import mediaurlhandler.mediaurlhandler

WEBSERVER = False
  
modules = [
    #circuits_testmodule.TestModule,
    #chathandler.ChatHandler,
    keycommands.KeyCommander,
    sleeper.Sleeper,
    #objectedit.objectedit.ObjectEdit, #now added to list at import due to new eror reporting tech
    #usr.anonlogin.AnonLogin,
    #mediaurlhandler.mediaurlhandler.MediaURLHandler,
    #loadurlhandler.LoadURLHandler,
    #apitest.pythonqt_gui.TestGui,
    #WebServer,
    #usr.mousecontrol.MouseControl,
    #apitest.testrunner.TestLoginLogoutExit,
]

#the error reporting here might be still better than in freshimport()
#- test and accommodate possible remaining feats from this block to there
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

#another exceptional import: failsafe loading of webserver
if WEBSERVER:
    try:
        import webserver.webcontroller
    except ImportError: #socket not avaible in debugmode
        print "NOTE: not enabling webserver 'cause socket module not available"
    else:
        from webserver.webcontroller import WebServer
        modules.append(WebServer)

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

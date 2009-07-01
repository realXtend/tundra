"""the modules that are to be loaded when the viewer starts.

add your module *class* (the circuits Component)
to the list called 'modules'. you need to import your pymodule
to have access here to the class, of course, and that can do
whatever loading that you want (in your module i.e. .py file).
"""

#import apitest.circuits_testmodule
import usr.chathandler
import usr.keycommands
#import usr.mousecontrol
#import usr.webserver

modules = [
    #apitest.circuits_testmodule.TestModule,
    usr.chathandler.ChatHandler,
    usr.keycommands.KeyCommander#,
    #usr.webserver.WebServer#,
    #usr.mousecontrol.MouseControl,
]

def load(circuitsmanager):
    for klass in modules:
        #modinst = klass()
        #circuitsmanager += modinst
        try:
            modinst = klass()
        except:
            print "failed to instansciate pymodule", klass #XXX add exception info
        else:
            circuitsmanager += modinst # Equivalent to: tm.register(m)
 
    #had trouble doing here too.. 
    #~ """an exception for the web server, had trouble doing this within it"""
    #~ ws = usr.webserver.WebServer()
    #~ rungen = (usr.webserver.Server(8000) + ws).run_once() #added a yield to the core
    #~ ws.rungen = rungen
    #~ circuitsmanager += ws


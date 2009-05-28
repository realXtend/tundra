"""the modules that are to be loaded when the viewer starts.

add your module *class* (the circuits Component)
to the list called 'modules'. you need to import your pymodule
to have access here to the class, of course, and that can do
whatever loading that you want (in your module i.e. .py file).
"""

import circuits_testmodule
import chathandler
import commander

modules = [
    circuits_testmodule.TestModule,
    chathandler.ChatHandler,
    commander.Commander
]

def load(circuitsmanager):
    for klass in modules:
        modinst = klass()
        circuitsmanager += modinst # Equivalent to: tm.register(m)

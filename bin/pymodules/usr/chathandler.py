"""was earlier direct test then modulemanager.py didn't exist,
and later with pyglet_manager a test with that, worked.
now the circuits_testmodule.TestModule has a chat handler too (as a method),
this is not called directly by the c++ side anymore,
but is to be loaded by the circuits_manager ModuleManager/ComponentRunner
"""

try:
    import rexviewer as r
except ImportError: #not running under rex
    import mockviewer as r

from circuits import Component

class ChatHandler(Component):
    def on_chat(self, frm, message):
        print "ChatHandler.onChat got chat message:", frm, message
        if 'bot' in message.lower():
            r.sendChat("I heard you!")
import rexviewer as r

"""
XXX NOTE: the current ScriptService / ScriptObject system,
as made for the CommunicationsUI and backend, works with 
classes only - is probably a good idea to allow any kind of 
py code, like straight functions, to be registered for calling
.. this worked in the earlier experiment with calling in pymodule
"""

def onChat(m):
    print "chathandler.onChat got chat message:", m
    if 'bot' in m.lower():
        r.sendChat("I heard you!")
    return True

class ChatHandler:
    """a hack to wrap the func in a class so ScriptObject CallMethod can work"""
    def onChat(self, m):
        return onChat(m)
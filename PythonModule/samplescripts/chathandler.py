import rexviewer as r

def onChat(m):
    print "chathandler.onChat got chat message:", m
    if 'bot' in m.lower():
        r.sendChat("I heard you!")
    return True
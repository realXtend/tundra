import rexviewer as r
from eventsource import viewer #the pyglet dispatcher

@viewer
def on_chat(frm, m):
    print "chathandler.onChat got chat message:", frm, m
    if 'bot' in m.lower():
        r.sendChat("I heard you!")
    return True

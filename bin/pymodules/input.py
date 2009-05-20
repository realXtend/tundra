import rexviewer as r

print dir(r)

#some prim
idnum = 720001
#idnum = 0

#av ent
av_entid = 8880005

idnum = av_entid

if 0: #get entity
    print "Getting entity id", idnum,
    e = r.getEntity(idnum)
    print "got:", e
    #print dir(r)

if 0: #test avatar tracking, works :)
    a = r.getEntity(av_entid)
    print "Avatar pos:", a.place
    """
    perhaps some local script could track movement?
    make a sound of a nearby object of interest, 
    like when a pet or a friend moves?
    """
    
if 1: #push an event, now an input event
    #from eventsource import viewer
    #from modulemanager import m 
    import circuits_manager
    mm = circuits_manager.ComponentRunner.instance
    print mm
    
    mm.INPUT_EVENT(r.MOVE_FORWARD_PRESSED)
    
    #a chat message again now too
    mm.RexNetMsgChatFromSimulator("Bob", "- that's me, Bob.")
    
    #previous pyglet stuff, was an ncoming chat msg event
    #m.dispatch_event('on_chat', "input", "testing")
    #print viewer._event_stack
    
    
    


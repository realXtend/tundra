import rexviewer as r

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
    
if 1: #push an incoming chat msg event
    from eventsource import viewer
    viewer.dispatch_event('on_chat', "input", "testing")
    print viewer._event_stack

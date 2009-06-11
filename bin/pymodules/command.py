import rexviewer as r

print "--- *** ---"

print dir(r)

#some prim
idnum = 720011 #the cube most far away from the screen in Toni & Petri 's test sim
#idnum = 0
new_id = 9999999

#av ent
av_entid = 8880007

idnum = av_entid

if 0: #get entity
    print "Getting entity id", idnum,
    e = r.getEntity(idnum)
    print "got:", e
    #print dir(r)

if 0: #test avatar tracking, works :)
    print "<:::",
    a = r.getEntity(av_entid)
    print "Avatar pos:", a.place,
    print ":::>"
    """
    perhaps some local script could track movement?
    make a sound of a nearby object of interest, 
    like when a pet or a friend moves?
    """
    
if 0: #push an event, now an input event
    #from eventsource import viewer
    #from modulemanager import m 
    import circuits_manager
    mm = circuits_manager.ComponentRunner.instance
    print mm
    
    mm.INPUT_EVENT(r.MoveForwardPressed)
    
    #a chat message again now too
    mm.RexNetMsgChatFromSimulator("Bob", "- that's me, Bob.")
    
    #previous pyglet stuff, was an ncoming chat msg event
    #m.dispatch_event('on_chat', "input", "testing")
    #print viewer._event_stack
    
if 1: #create entity
    pass
    #not safe now:
    """
    New entity created:16:39:22 [Foundation] Error: Can't create entity with given i
d because it's already used: 9999999
Assertion failed: px != 0, file D:\k2\rex\viewer\trunk\external_libs\include\boo
st/shared_ptr.hpp, line 419
    """
    #print "New entity created:", r.createEntity(new_id)
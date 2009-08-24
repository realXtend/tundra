"""this is executed when you press '.' in the ogre window, the viewer main window.
used for quick testing of py commands."""

import rexviewer as r

print "--- *** ---"

print dir(r)

#some prim
idnum = 720011 #the cube most far away from the screen in Toni & Petri 's test sim
#idnum = 0
new_id = 9999999

#av ent
av_entid = 8880000

def rotate(e):
    o = e.orientation    
    newort = (o[0], o[1], o[2] + 0.5, o[3])
    print "Rotating to ort:", newort
    e.orientation = newort
    #assert e.orientation[2] > (oldz+0.9) #xxx some logic fail here?
    #print "TEST ORIENTATION SUCCEEDED", e.orientation[2], oldortz
    
def move(e):
    p = e.pos #.pos - the w.i.p. api has a shortcut now that instead of a placeable with loc,rot,scale it just gives loc now directly
    oldx = p[0] #p.x - Vector3 not wrapped (yet), just gives a tuple
    #p.x += 1 #change the x-coordinate
    newpos = (p[0] - 1, p[1], p[2])
    print "Moving to move to pos:", newpos
    e.pos = newpos

if 0:
    print "Testing camera swap..."
    hmm = r.switchCameraState()
    #print hmm
    
if 0: #get entity
    #idnum = new_id
    print "Getting entity id", idnum,
    e = r.getEntity(idnum)
    print "got:", e
    #print dir(r)
    rotate(e)
    #move(e)

if 0: #test avatar tracking, works :)
    print "<:::",
    try:
        a = r.getEntity(av_entid)
    except:
        print "could find the avatar with the given id", av_entid
    else:
        print "Avatar pos:", a.pos,
        print ":::>"
        """
        perhaps some local script could track movement?
        make a sound of a nearby object of interest, 
        like when a pet or a friend moves?
        """
        
        #test what happens when we move the av
        #a.pos = a.pos[0] + 1, a.pos[1], a.pos[2]
        """crash, because of how network updates are coded in the internals: 
        XXX RexServerConnecion.cpp
        RexServerConnection::SendMultipleObjectUpdatePacket(std::vector<Scene::EntityPtr> entity_ptr_list)
        const Foundation::ComponentInterfacePtr &prim_component = entity_ptr_list[i]->GetComponent("EC_OpenSimPrim");
        (because avatars don't have the prim component"""
        
        #rotating the av
        rotate(a)

if 0: #push an event, input and/or chat
    #from eventsource import viewer
    #from modulemanager import m 
    import circuits_manager
    mm = circuits_manager.ComponentRunner.instance
    print mm
    
    #mm.INPUT_EVENT(r.MoveForwardPressed)
    
    #a chat message again now too
    mm.RexNetMsgChatFromSimulator("Bob", "- that's me, Bob.")
    
    #previous pyglet stuff, was an ncoming chat msg event
    #m.dispatch_event('on_chat', "input", "testing")
    #print viewer._event_stack
    
if 0: #create entity
    pass
    #not safe now:
    """
    New entity created:16:39:22 [Foundation] Error: Can't create entity with given i
d because it's already used: 9999999
Assertion failed: px != 0, file D:\k2\rex\viewer\trunk\external_libs\include\boo
st/shared_ptr.hpp, line 419
    """
    #print "New entity created:", r.createEntity(new_id)

if 0: #placeable and text tests
    print "Testing..."
    e = r.getEntity(720005)    

    #~ try:
        #~ e.pos = 1
    #~ except Exception, e:
        #~ print e
        
    #e.orientation = "well this ain't a quarternion."
    #e.scale = ("does", "this", "work")
    def test():
        print "this ain't a string..."
    
    e.text = "swoot"
    e.text = 1
    e.text = ("swoot", "ness")
    e.text = (1, 2)
    e.text = test
    
    e.pos = 1
    
    print e.text, e.pos, e.scale, e.orientation
    
if 0: #send chat
    r.sendChat("here we go.")
    #print "called sendchat ok"
    
if 0: #camera pitch
    dy = 0.1
    dp = 0
    #dp = -0.1
    #dp = 0.1
    r.setCameraYawPitch(dy, dp)
    print r.getCameraYawPitch()
        
if 0: #avatar set yaw (turn)
    r.setAvatarYaw(-1)
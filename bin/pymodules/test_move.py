"""
adopted from PythonScriptModule/proto/entity.py
where first wrote a similar test and prototyped the impl
which has since been made within the actual viewer using the py c api.

for documentation and design notes see that file,
this has notes about the current state of the actual api impl.

the intended ways that are not implemented yet are commented out,
they work in the py-written prototype,
and the ones that work with the current actual api are run here.
"""

import rexviewer as r

idnum = 720004
#idnum = 0

#playing with avatar
#av_entid = 8880004
#idnum = av_entid

def test_move(e):
    p = e.pos #.pos - the w.i.p. api has a shortcut now that instead of a placeable with loc,rot,scale it just gives loc now directly
    oldx = p[0] #p.x - Vector3 not wrapped (yet), just gives a tuple
    #p.x += 1 #change the x-coordinate
    newpos = (p[0] + 1, p[1], p[2])
    print "TEST MOVE: trying to move to pos:", newpos
    e.pos = newpos
    
    assert e.pos[0] > (oldx + 0.9) #and ,finally test if it actually did move
    print "TEST MOVE SUCCEEDED", e.pos[0], oldx #if we get this far, the move actually worked! Yay.

def test_scale(e):
    p = e.scale
    oldx = p[0]
    
    newscale = (p[0]+1, p[1], p[2])
    print "TEST SCALE: trying to scale the avatar to:", newscale
    e.scale = newscale
    assert e.scale[0] > (oldx+0.9)
    print "TEST SCALE SUCCEEDED", e.scale[0], oldx
    
def test_orientation(e):
    p = e.orientation
    oldz = p[2]
    
    newort = (p[0], p[1], p[2]+1, p[3])
    print "TEST ORIENTATION: trying to rotate the avatar around its axis to:", newort
    e.orientation = newort
    assert e.orientation[2] > (oldz+0.9)
    print "TEST ORIENTATION SUCCEEDED", e.orientation[2], oldz
    
def runtests():
    #e = viewer.scenes['World'].entities[1] #the id for testdummy, or by name?
    #note: now is directly in viewer as GetEntity, defaulting to 'World' scene
    
    e = r.getEntity(idnum)
    test_move(e)
    test_scale(e)
    test_orientation(e)
    
runtests()

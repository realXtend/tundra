"""a module to move something in the scene with the mouse.
is a use case that was identified when gathering api reqs,
this impl made as a test whether putting the pieces together actually works,
now that entity moving and mouse event getting works.

for the use case in mind the entity moved here actually would be a local
cursor thing, towards which the avatar moves (rts style). testing with a prim now.
"""

try:
    import rexviewer as r
except ImportError: #not running under rex
    import mockviewer as r
from circuits import Component

idnum = 720006

class MouseControl(Component):
    def __init__(self):
        Component.__init__(self)
        
        #XXX should done this only after connecting or something
        self.target = r.getEntity(idnum)
        print "MouseControl inited with target", self.target
        
    def on_mousemove(self, m):
        e = self.target
        print m
        print "moved from", e.pos, "to ->",
        x, y, z = e.pos #should probably wrap Vector3, see test_move.py for refactoring notes        
        x += m.rel_x #for the actual use case would need birds eye cam and raycast to ground towards the mouse cursor pos, this was kinda fun too though :)
        y += m.rel_y
        e.pos = x, y, z
        print e.pos

"""
A gui tool for editing.

Now a basic proof-of-concept and a test of the Python API:
The qt integration for ui together with the manually wrapped entity-component data API,
and the viewer non-qt event system for mouse events thru the py plugin system.

Works for selecting an object with the mouse, and then changing the position 
using the qt widgets. Is shown immediately in-world and synched over the net.

TODO (most work is in api additions on the c++ side, then simple usage here):
- show name of the selected object
- sync changes from over the net to the gui dialog: listen to scene objectupdates
  (this is needed/nice when someone else is moving the same obj at the same time,
   works correctly in slviewer, i.e. the dialogs there update on net updates)
- mousemove always, all button clicks
- hilite the selected object
(- list all the objects to allow selection from there)


"""

import rexviewer as r
import PythonQt
from circuits import Component

class EditGUI(Component):
    EVENTHANDLED = False
    
    def __init__(self):
        Component.__init__(self)
        
        canvas = r.createCanvas()
        #now test code does hardcoded test loadui itself
        widget = canvas
        #widget = loadui("dialog.ui");
        #canvas.addWidget(widget)
        
        self.widget = widget
        #for some reason setRange is not there. is not not a slot of these?
        #"QDoubleSpinBox has no attribute named 'setRange'"
        #apparently they are properties .minimum and .maximum, made in the xml now
        #~ for p in [widget.xpos, widget.ypos]:
            #~ p.setRange(0, 256)
        #~ widget.zpos.setRange(0, 100)
        
        #~ widget.xpos.connect('valueChanged(double)', self.changed_x)
        def poschanger(i):
            def pos_at_index(v):
                self.changepos(i, v)
            return pos_at_index
        for i, poswidget in enumerate([widget.xpos, widget.ypos, widget.zpos]):
            #poswidget.connect('valueChanged(double)', lambda v: self.changepos(i, v))
            poswidget.connect('valueChanged(double)', poschanger(i))
        
        self.sel = None
        
        self.left_button_down = False
        
    #~ def changed_x(self, v):
        #~ print "x changed to: %f" % v
        #~ ent = self.sel
        #~ if ent is not None:
            #~ print "sel pos is:", ent.pos
            #~ _, y, z = ent.pos #should probably wrap Vector3, see test_move.py for refactoring notes        
            #~ ent.pos = v, y, z
                        
    def changepos(self, i, v):
        #XXX NOTE / API TODO: exceptions in qt slots (like this) are now eaten silently
        #.. apparently they get shown upon viewer exit. must add some qt exc thing somewhere
        #print "pos index %i changed to: %f" % (i, v)
        ent = self.sel
        if ent is not None:
            #print "sel pos:", ent.pos,
            pos = list(ent.pos) #should probably wrap Vector3, see test_move.py for refactoring notes. 
            #converted to list to have it mutable
            pos[i] = v
            ent.pos = pos[0], pos[1], pos[2] #XXX API should accept a list/tuple too .. or perhaps a vector type will help here too
            #print "=>", ent.pos
            
    def select(self, ent):
        self.sel = ent
        print "Selected entity:", self.sel.id, "at", self.sel.pos#, self.sel.name
        
        #update the gui vals to show what the newly selected entity has
        x, y, z = ent.pos
        self.widget.xpos.setValue(x)
        self.widget.ypos.setValue(y)
        self.widget.zpos.setValue(z)
        
    def on_mousemove(self, mouseinfo):
        """stub for dragging objects around 
        - should get the dir of movements relative to the view somehow"""
        if self.left_button_down and self.sel is not None:
            print "MouseMove:", mouseinfo.x, mouseinfo.y

    def on_mouseclick(self, click_id, mouseinfo):
        #print "MouseMove", mouseinfo.x, mouseinfo.y
        if click_id == r.LeftMouseClickPressed:
            self.left_button_down = True
            ent = r.rayCast(mouseinfo.x, mouseinfo.y)
            #print "Got entity:", ent
            if ent is not None:
                r.eventhandled = self.EVENTHANDLED
                #print "Entity position is", ent.pos
                #print "Entity id is", ent.id
                #if self.sel is not ent: #XXX wrappers are not reused - there may now be multiple wrappers for same entity
                if self.sel is None or self.sel.id != ent.id: #a diff ent than prev sel was changed
                    self.select(ent)
        elif click_id == r.LeftMouseClickReleased:
            self.left_button_down = False
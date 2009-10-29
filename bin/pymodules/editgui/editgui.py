"""
A gui tool for editing.

Now a basic proof-of-concept and a test of the Python API:
The qt integration for ui together with the manually wrapped entity-component data API,
and the viewer non-qt event system for mouse events thru the py plugin system.

Works for selecting an object with the mouse, and then changing the position 
using the qt widgets. Is shown immediately in-world and synched over the net.

TODO (most work is in api additions on the c++ side, then simple usage here):
- (WIP, needs network event refactoring) sync changes from over the net to the gui dialog: listen to scene objectupdates
  (this is needed/nice when someone else is moving the same obj at the same time,
   works correctly in slviewer, i.e. the dialogs there update on net updates)
- hilite the selected object
(- list all the objects to allow selection from there)


"""

import rexviewer as r
import PythonQt
from PythonQt.QtGui import QTreeWidgetItem
from PythonQt.QtUiTools import QUiLoader
from PythonQt.QtCore import QFile
from circuits import Component

from conversions import * #for euler - quat -euler conversions

try:
    import ogre.renderer.OGRE as ogre
except ImportError:
    ogreroot = False
else:
    root = ogre.Root.getSingleton()
    ogreroot = root.isInitialised()
    V3 = ogre.Vector3
    
INTERNAL = 1
EXTERNAL = 0

UP = 0
RIGHT = 1

class EditGUI(Component):
    EVENTHANDLED = False
    OIS_ESC = 1
    UIFILE = "pymodules/editgui/editobject.ui"
    
    def __init__(self):
        Component.__init__(self)
        loader = QUiLoader()
        self.canvas = r.createCanvas(EXTERNAL) #change to internal later, had some rendering problems?
        file = QFile(self.UIFILE)

        widget = loader.load(file)
        width = widget.size.width()
        height = widget.size.height()
        self.canvas.resize(width, height)
        self.canvas.AddWidget(widget)
        #self.canvas.Show()
        modu = r.getQtModule()
        modu.AddCanvasToControlBar(self.canvas, "EditGUI")
        
        #self.deactivate()
        
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

        def rotchanger(i):
            def rot_at_index(v):
                self.changerot(i, v)
            return rot_at_index
        for i, rotwidget in enumerate([widget.rot_x, widget.rot_y, widget.rot_z]):
            rotwidget.connect('valueChanged(double)', rotchanger(i))
        
        def scalechanger(i):
            def scale_at_index(v):
                self.changescale(i, v)
            return scale_at_index
        for i, scalewidget in enumerate([widget.scalex, widget.scaley, widget.scalez]):
            scalewidget.connect('valueChanged(double)', scalechanger(i))
        
        self.sel = None
        
        self.left_button_down = False
        self.right_button_down = False
        self.widget = widget
        self.widget.label.text = "<none>"
        
        r.c = self
        self.widget.treeWidget.connect('clicked()', self.itemActivated)
        #self.widget.treeWidget.connect('activated(QModelIndex)', self.itemActivated)
        
        self.widgetList = {}
        
        self.cam = None
        
        try:
            self.upArrow = r.arrows[UP]
        except: 
            self.upArrow = None
            
        try:
            self.rightArrow = r.arrows[RIGHT]            
        except:
            self.rightArrow = None
        
        self.mouse_events = {
            r.LeftMouseClickPressed: self.LeftMouseDown,
            r.LeftMouseClickReleased: self.LeftMouseUp,  
            r.RightMouseClickPressed: self.RightMouseDown,
            r.RightMouseClickReleased: self.RightMouseUp
        }
        
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
    
    def changescale(self, i, v):
        ent = self.sel
        if ent is not None:
            scale = list(ent.scale)
            scale[i] = v
            ent.scale = scale[0], scale[1], scale[2]
    
    def changerot(self, i, v):
        #XXX NOTE / API TODO: exceptions in qt slots (like this) are now eaten silently
        #.. apparently they get shown upon viewer exit. must add some qt exc thing somewhere
        #print "pos index %i changed to: %f" % (i, v)
        ent = self.sel
        if ent is not None:
            #print "sel orientation:", ent.orientation
            #from euler x,y,z to to quat
            euler = list(quat_to_euler(ent.orientation))
            euler[i] = v
            ort = euler_to_quat(euler)
            #print euler, ort
            #print euler, ort
            ent.orientation = ort
            
    def itemClicked(self): #XXX dirty hack to get single click for activating, should we use some qt stylesheet for this, or just listen to click?
        self.itemActivated(self) 
    
    def itemActivated(self, item=None): #the item from signal is not used, same impl used by click
        #print "Got the following item index...", item, dir(item), item.data, dir(item.data) #we has index, now what? WIP
        current = self.widget.treeWidget.currentItem()
        text = current.text(0)
        print "Selected:", text
        if self.widgetList.has_key(text):
            self.select(self.widgetList[text][0])
    
    def select(self, ent):
        if ent.id != 0:
            self.sel = ent
            
            if not self.widgetList.has_key(self.sel.id):
                tWid = QTreeWidgetItem(self.widget.treeWidget)
                id = self.sel.id
                tWid.setText(0, id)
                
                self.widgetList[str(id)] = (ent, tWid)

            print "Selected entity:", self.sel.id, "at", self.sel.pos#, self.sel.name

            #update the gui vals to show what the newly selected entity has
            x, y, z = ent.pos
            self.widget.xpos.setValue(x)
            self.widget.ypos.setValue(y)
            self.widget.zpos.setValue(z)
            
            x, y, z = ent.scale
            self.widget.scalex.setValue(x)
            self.widget.scaley.setValue(y)
            self.widget.scalez.setValue(z)
            
            #from quat to euler x.y,z
            euler = quat_to_euler(ent.orientation)
            #print euler
            self.widget.rot_x.setValue(euler[0])
            self.widget.rot_y.setValue(euler[1])
            self.widget.rot_z.setValue(euler[2])
            
            self.widget.label.text = ent.id
            
            if self.cam is None and ogreroot:
                rs = root.getRenderSystem()
                vp = rs._getViewport()
                self.cam = vp.getCamera()
                self.drawArrows(ent)
            elif self.cam is not None and ogreroot: 
                self.drawArrows(ent)
    
    def drawArrows(self, ent):
        #~ right = self.cam.getRight().normalisedCopy()
        #~ up = self.cam.getUp().normalisedCopy()
        #~ #print right, up, ent.pos
        x, y, z = ent.pos
        pos = V3(x, y, z)
        
        #~ rightArrowEnd = pos + right *5 
        #~ upArrowEnd = pos + up *5 
        #~ #print "Directions", rightDir, upDir
        #~ #print "Arrow end positions:", rightArrowEnd, upArrowEnd
        
        if self.upArrow is None: #creating the upArrow for the first time, well only time it should be created really
            self.upArrow = self.createArrow(UP, V3(0, 0, 0), V3(0, 5, 0))
        
        if self.rightArrow is None:
            self.rightArrow = self.createArrow(RIGHT, V3(0, 0, 0), V3(5, 0, 0))
    
        #print self.upArrow, type(self.upArrow), "\n"
        #print self.rightArrow, type(self.rightArrow)
        
        self.showArrow(self.upArrow, pos)
        self.showArrow(self.rightArrow, pos)
        
    def createArrow(self, direction, start, end):
        sm = root.getSceneManager("SceneManager")
        mob =  sm.createManualObject("arrow_EditGui_MOB_%d" % direction)
        arrow = sm.getRootSceneNode().createChildSceneNode("arrow_EditGui_NODE_%d" % direction)

        material = ogre.MaterialManager.getSingleton().create("manual1Material","debugger")
        material.setReceiveShadows(False)
        tech = material.getTechnique(0)
        tech.setLightingEnabled(True)
        pass0 = tech.getPass(0)
        pass0.setDiffuse(0, 0, 1, 0)
        pass0.setAmbient(0, 0, 1)
        pass0.setSelfIllumination(0, 0, 1)
        
        mob.begin("manual1Material", ogre.RenderOperation.OT_LINE_LIST)
        mob.position(start)
        mob.position(end)
        mob.end()
        mob.setVisible(True)
        arrow.setVisible(True)
        print "created the manual material"
        arrow.attachObject(mob)
        r.arrows[direction] = arrow
        return arrow
        
    def showArrow(self, arrow, pos):
        arrow.setPosition(pos)
        arrow.setOrientation(self.cam.DerivedOrientation)
        arrow.setVisible(True)

    def hideArrows(self):
        #print "Hiding arrows!"
        
        if self.upArrow is not None:
            self.upArrow.setVisible(False)
    
        if self.rightArrow is not None:
            self.rightArrow.setVisible(False)
            

            
    def LeftMouseDown(self, mouseinfo):
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
        else:
            self.sel = None
            self.widget.label.text = "<none>"
            
    def LeftMouseUp(self, mouseinfo):
        self.left_button_down = False
        
    def RightMouseDown(self, mouseinfo):
        self.right_button_down = True
        
    def RightMouseUp(self, mouseinfo):
        self.right_button_down = False
        
    def on_mouseclick(self, click_id, mouseinfo, callback):
        #print "MouseMove", mouseinfo.x, mouseinfo.y, self.canvas.IsHidden()
        #print "on_mouseclick", click_id,
        if not self.canvas.IsHidden():
            if self.mouse_events.has_key(click_id):
                self.mouse_events[click_id](mouseinfo)
                #print "on_mouseclick", click_id, self.mouse_events[click_id]
            #else:
                #print "unknown click_id?", self.mouse_events
            
    def on_mousemove(self, mouseinfo, callback):
        """stub for dragging objects around 
        - should get the dir of movements relative to the view somehow"""
        return
        if not self.canvas.IsHidden():
            if self.left_button_down and self.sel is not None:
                print "MouseMove:", mouseinfo.x, mouseinfo.y

            elif self.right_button_down and self.sel is not None:
                self.rightArrow.setOrientation(self.cam.DerivedOrientation)
                self.upArrow.setOrientation(self.cam.DerivedOrientation)

    def on_exit(self):
        r.logInfo("EditGUI exiting.")
        self.hideArrows()
        
        modu = r.getQtModule()
        if self.canvas is not None:
            modu.DeleteCanvas(self.canvas)
            
    #~ def activate(self):
        #~ self.activated = True
        #~ self.canvas.Show()

    #~ def deactivate(self):
        #~ self.activated = False
        #~ self.canvas.Hide()




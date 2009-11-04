"""
A gui tool for editing.

Now a basic proof-of-concept and a test of the Python API:
The qt integration for ui together with the manually wrapped entity-component data API,
and the viewer non-qt event system for mouse events thru the py plugin system.

Works for selecting an object with the mouse, and then changing the position 
using the qt widgets. Is shown immediately in-world and synched over the net.

TODO (most work is in api additions on the c++ side, then simple usage here):
- local & global movement - select box?
- (WIP, needs network event refactoring) sync changes from over the net to the gui dialog: listen to scene objectupdates
  (this is needed/nice when someone else is moving the same obj at the same time,
   works correctly in slviewer, i.e. the dialogs there update on net updates)
- hilite the selected object
(- list all the objects to allow selection from there)


"""

import rexviewer as r
import PythonQt
from PythonQt.QtGui import QTreeWidgetItem, QInputDialog, QLineEdit
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
            
        self.canvas = r.createCanvas(INTERNAL) #change to internal later, had some rendering problems?
        self.arrows = None
        
        file = QFile(self.UIFILE)

        widget = loader.load(file)
        width = widget.size.width()
        height = widget.size.height()

        self.canvas.SetCanvasSize(width, height)
        self.canvas.SetPosition(30, 30)
        self.canvas.SetCanvasResizeLock(True)

        widget.resize(width, height)

        self.canvas.AddWidget(widget)
        #self.canvas.Show()
        modu = r.getQtModule()
        modu.AddCanvasToControlBar(self.canvas, "World Edit")

        #self.deactivate()
        
        #for some reason setRange is not there. is not not a slot of these?
        #"QDoubleSpinBox has no attribute named 'setRange'"
        #apparently they are properties .minimum and .maximum, made in the xml now
        #~ for p in [widget.MainFrame.xpos, widget.MainFrame.ypos]:
            #~ p.setRange(0, 256)
        #~ widget.zpos.setRange(0, 100)
        
        #~ widget.xpos.connect('valueChanged(double)', self.changed_x)
        def poschanger(i):
            def pos_at_index(v):
                self.changepos(i, v)
            return pos_at_index
        for i, poswidget in enumerate([widget.MainFrame.xpos, widget.MainFrame.ypos, widget.MainFrame.zpos]):
            #poswidget.connect('valueChanged(double)', lambda v: self.changepos(i, v))  
            poswidget.connect('valueChanged(double)', poschanger(i))

        def rotchanger(i):
            def rot_at_index(v):
                self.changerot(i, v)
            return rot_at_index
        for i, rotwidget in enumerate([widget.MainFrame.rot_x, widget.MainFrame.rot_y, widget.MainFrame.rot_z]):
            rotwidget.connect('valueChanged(double)', rotchanger(i))
        
        def scalechanger(i):
            def scale_at_index(v):
                self.changescale(i, v)
            return scale_at_index
        for i, scalewidget in enumerate([widget.MainFrame.scalex, widget.MainFrame.scaley, widget.MainFrame.scalez]):
            scalewidget.connect('valueChanged(double)', scalechanger(i))
        
        self.sel = None
        
        self.left_button_down = False
        self.right_button_down = False
        self.widget = widget.MainFrame
        self.widget.label.text = "<none>"
        
        r.c = self

        self.widget.treeWidget.connect('clicked(QModelIndex)', self.itemActivated)
        #self.widget.treeWidget.connect('activated(QModelIndex)', self.itemActivated)
        
        self.widget.newObject.connect('clicked()', self.createObject)
        self.widget.setMesh.connect('clicked()', self.setMesh)
        
        self.widgetList = {}
        
        self.cam = None
        
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
            if self.arrows is not None:
                self.arrows.pos = pos[0], pos[1], pos[2]
    
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
            
            if self.arrows is not None:
                self.arrows.orientation = ort
            
    def itemActivated(self, item=None): #the item from signal is not used, same impl used by click
        #print "Got the following item index...", item, dir(item), item.data, dir(item.data) #we has index, now what? WIP
        current = self.widget.treeWidget.currentItem()
        text = current.text(0)
        #print "Selected:", text
        if self.widgetList.has_key(text):
            self.select(self.widgetList[text][0])
    
    def createObject(self, *args):
        ent_id = r.getUserAvatarId()
        ent = r.getEntity(ent_id)
        x, y, z = ent.pos#r.getUserAvatarPos()

        start_x = x
        start_y = y
        start_z = z
        end_x = x
        end_y = y
        end_z = z

        r.sendObjectAddPacket(start_x, start_y, start_z, end_x, end_y, end_z)

    def setMesh(self, *args):
        meshUUID = None
        meshUUID = QInputDialog.getText(None, "Mesh asset UUID", "Please give mesh asset UUID", QLineEdit.Normal, "")
        if meshUUID != "" and meshUUID != None:
            print "User gave mesh asset UUID of " + meshUUID
            # validate UUID somehow and get selected item from listview -> put mesh to it with id :)
            #XXX validate
            self.sel.mesh = meshUUID
            r.sendRexPrimData(self.sel.id)

    def select(self, ent):
        arrows = False
        if self.arrows is not None and self.arrows.id == ent.id:
            arrows = True
            
        if ent.id != 0 and ent.id != r.getUserAvatarId() and not arrows:
            self.sel = ent
            
            if not self.widgetList.has_key(str(self.sel.id)):
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
            
            #~ if self.cam is None and ogreroot:
                #~ rs = root.getRenderSystem()
                #~ vp = rs._getViewport()
                #~ self.cam = vp.getCamera()
                #~ self.drawArrows(ent)
            #~ elif self.cam is not None and ogreroot: 
                #~ self.drawArrows(ent)
            #~ else:
            
            self.drawArrows(ent) #causes crash at quit, so disabled for now, uncomment for testing
    
    def drawArrows(self, ent):
        #print "drawArrows", self.arrows
        x, y, z = ent.pos
        pos = x, y, z
        
        sx, sy, sz = ent.scale
        scale = sx, sy, sz
        
        ox, oy, oz, ow = ent.orientation
        ort = ox, oy, oz, ow
        
        if self.arrows is None:
            self.arrows = self.createArrows()
        
        self.showArrow(pos, scale, ort)
        
    def createArrows(self):
        #print "\nCreating arrows!\n"
        ent = r.createEntity("axes.mesh")
        return ent
        
    def showArrow(self, pos, scale, ort):
        #print "Showing arrows!"
        if self.arrows is not None:
            self.arrows.pos = pos
            self.arrows.scale = 0.2, 0.2, 0.2
            #self.arrow.setOrientation(self.cam.DerivedOrientation)
            self.arrows.orientation = ort
        
    def hideArrows(self):
        #print "Hiding arrows!"
        if self.arrows is not None:
            self.arrows.scale = 0.0, 0.0, 0.0 #ugly hack
            self.arrows.pos = 0.0, 0.0, 0.0 #another ugly hack
            #XXX todo: change these after theres a ent.hide type way

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
        
        #out to ease dev now that mouse clicks go 'thru' the qt canvas
        else:
            self.sel = None
            self.widget.label.text = "<none>"
            self.hideArrows()
            
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
            #print "Point!"
            if self.mouse_events.has_key(click_id):
                self.mouse_events[click_id](mouseinfo)
                #print "on_mouseclick", click_id, self.mouse_events[click_id]
            #else:
                #print "unknown click_id?", self.mouse_events
            
    def on_mousemove(self, mouseinfo, callback):
        """stub for dragging objects around 
        - should get the dir of movements relative to the view somehow"""
        if not self.canvas.IsHidden():
            if self.left_button_down and self.sel is not None:
                print "MouseMove:", mouseinfo.x, mouseinfo.y, r.getCameraUp(), r.getCameraRight()

    def on_exit(self):
        r.logDebug("EditGUI exiting...")
        
        if r.restart:
            #r.logDebug("...restarting...")
            self.hideArrows()
        
        modu = r.getQtModule()
        if self.canvas is not None:
            modu.DeleteCanvas(self.canvas)

        r.logDebug("   ...exit done.")

#barrel on 0.5 in viila: 
# Upload succesfull. Asset id: 35da6174-8743-4026-a83e-18b23984120d, 
# inventory id: 12c3df2d-ef3b-490e-8615-2f89abb7375d.



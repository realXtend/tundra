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
from vector3 import Vector3 #for view based editing calcs now that Vector3 not exposed from internals

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

DEV = True #if this is false, the canvas is added to the controlbar

class MeshAssetidEditline(QLineEdit):
    def __init__(self, mainedit, *args):
        self.mainedit = mainedit #to be able to query the selected entity at drop
        QLineEdit.__init__(self, *args)

    def dropEvent(self, ev):
    #    print "Got meshid_drop:", self, ev
        text = ev.mimeData().text()
        r.logDebug("EDITGUI: Mesh asset id got drag drop: %s" % text)
        ent = self.mainedit.sel #is public so no need for getter, can be changed to a property if needs a getter at some point
        #XXX validate input for being a valid mesh asset UUID somehow
        if ent is not None:
            applymesh(ent, text)
            self.text = text
        else:
            self.text = "(no scene entity selected)"

def applymesh(ent, meshuuid):
    ent.mesh = meshuuid
    #r.logDebug("Mesh asset UUID after before sending to server: %s" % ent.mesh)
    r.sendRexPrimData(ent.id)
    r.logDebug("Mesh asset UUID after prim data sent to server: %s" % ent.mesh)

class EditGUI(Component):
    EVENTHANDLED = False
    OIS_ESC = 1
    UIFILE = "pymodules/editgui/editobject.ui"
    
    def __init__(self):
        Component.__init__(self)
        loader = QUiLoader()
            
        self.canvas = r.createCanvas(EXTERNAL) #now for drag&drop dev
        self.arrows = None
        
        uifile = QFile(self.UIFILE)

        ui = loader.load(uifile)
        width = ui.size.width()
        height = ui.size.height()

        self.canvas.SetSize(width, height)
        self.canvas.SetPosition(30, 30)
        self.canvas.SetResizable(False)

        ui.resize(width, height)

        self.canvas.AddWidget(ui)
        self.widget = ui.MainFrame
        self.widget.label.text = "<none>"

        #print dir(ui)
        meshassetedit = MeshAssetidEditline(self) #ui) #ui.MainFrame) - crashes :o
        """<x>190</x>
        <y>180</y>
        <width>151</width>
        <height>20</height>"""
        meshassetedit.show()
        meshassetedit.move(190, 180)
        self.canvas.AddWidget(meshassetedit)
        
        self.canvas.connect('Hidden()', self.on_hide)
        modu = r.getQtModule()
        if not DEV:
            modu.AddCanvasToControlBar(self.canvas, "World Edit")
        else:
            self.canvas.Show()
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
        for i, poswidget in enumerate([ui.MainFrame.xpos, ui.MainFrame.ypos, ui.MainFrame.zpos]):
            #poswidget.connect('valueChanged(double)', lambda v: self.changepos(i, v))  
            poswidget.connect('valueChanged(double)', poschanger(i))

        def rotchanger(i):
            def rot_at_index(v):
                self.changerot(i, v)
            return rot_at_index
        for i, rotwidget in enumerate([ui.MainFrame.rot_x, ui.MainFrame.rot_y, ui.MainFrame.rot_z]):
            rotwidget.connect('valueChanged(double)', rotchanger(i))
        
        def scalechanger(i):
            def scale_at_index(v):
                self.changescale(i, v)
            return scale_at_index
        for i, scalewidget in enumerate([ui.MainFrame.scalex, ui.MainFrame.scaley, ui.MainFrame.scalez]):
            scalewidget.connect('valueChanged(double)', scalechanger(i))
        
        self.sel = None
        
        self.left_button_down = False
        self.right_button_down = False
                
        self.widget.treeWidget.connect('clicked(QModelIndex)', self.itemActivated)
        #self.widget.treeWidget.connect('activated(QModelIndex)', self.itemActivated)
        
        self.widget.newObject.connect('clicked()', self.createObject)
        self.widget.deleteObject.connect('clicked()', self.deleteObject)
        self.widget.setMesh.connect('clicked()', self.setMesh)
        
        self.widgetList = {}
        
        self.cam = None
        
        self.mouse_events = {
            r.LeftMouseClickPressed: self.LeftMouseDown,
            r.LeftMouseClickReleased: self.LeftMouseUp,  
            r.RightMouseClickPressed: self.RightMouseDown,
            r.RightMouseClickReleased: self.RightMouseUp
        }
        
        self.arrow_grabbed = False
        self.arrow_grabbed_axis = None

    #r.c = self

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
            r.networkUpdate(ent.id)
            #print "=>", ent.pos
            if self.arrows is not None:
                self.arrows.pos = pos[0], pos[1], pos[2]
    
    def changescale(self, i, v):
        ent = self.sel
        if ent is not None:
            scale = list(ent.scale)
            scale[i] = v
            ent.scale = scale[0], scale[1], scale[2]
            r.networkUpdate(ent.id)
    
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
            r.networkUpdate(ent.id)
            
            if self.arrows is not None:
                self.arrows.orientation = ort
            
    def itemActivated(self, item=None): #the item from signal is not used, same impl used by click
        #print "Got the following item index...", item, dir(item), item.data, dir(item.data) #we has index, now what? WIP
        current = self.widget.treeWidget.currentItem()
        text = current.text(0)
        #print "Selected:", text
        if self.widgetList.has_key(text):
            self.select(self.widgetList[text][0])
    
    def createObject(self):
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

    def deleteObject(self):
        pass #not implemented yet, this is a stub waiting for impl
        #ent = self.sel
        #if ent is not None:
        #    r.sendObjectRemovePacket(ent.id)

    def setMesh(self):
        """callback for the original set mesh button, going away."""
        meshUUID = None
        meshUUID = QInputDialog.getText(None, "Mesh asset UUID", "Please give mesh asset UUID", QLineEdit.Normal, "")
        if meshUUID != "" and meshUUID != None:
            r.logDebug("User gave mesh asset UUID %s" % meshUUID)
            # validate UUID somehow and get selected item from listview -> put mesh to it with id :)
            #XXX validate
            #print "Type of meshUUID:", type(meshUUID)
            applymesh(self.sel, meshUUID)

    def select(self, ent):
        arrows = False
        if self.arrows is not None and self.arrows.id == ent.id:
            arrows = True

        if ent.id != 0 and ent.id > 10 and ent.id != r.getUserAvatarId() and not arrows: #terrain seems to be 3 and scene objects always big numbers, so > 10 should be good
            self.sel = ent
            
            if not self.widgetList.has_key(str(self.sel.id)):
                tWid = QTreeWidgetItem(self.widget.treeWidget)
                id = self.sel.id
                tWid.setText(0, id)
                
                self.widgetList[str(id)] = (ent, tWid)

            #print "Selected entity:", self.sel.id, "at", self.sel.pos#, self.sel.name

            #update the gui vals to show what the newly selected entity has
            self.update_guivals()
            self.widget.label.text = ent.id

    def update_guivals(self):
        x, y, z = self.sel.pos
        self.widget.xpos.setValue(x)
        self.widget.ypos.setValue(y)
        self.widget.zpos.setValue(z)
            
        x, y, z = self.sel.scale
        self.widget.scalex.setValue(x)
        self.widget.scaley.setValue(y)
        self.widget.scalez.setValue(z)
            
        #from quat to euler x.y,z
        euler = quat_to_euler(self.sel.orientation)
        #print euler
        self.widget.rot_x.setValue(euler[0])
        self.widget.rot_y.setValue(euler[1])
        self.widget.rot_z.setValue(euler[2])        
            
            #~ if self.cam is None and ogreroot:
                #~ rs = root.getRenderSystem()
                #~ vp = rs._getViewport()
                #~ self.cam = vp.getCamera()
                #~ self.drawArrows(ent)
            #~ elif self.cam is not None and ogreroot: 
                #~ self.drawArrows(ent)
            #~ else:
            
        self.drawArrows(self.sel) #causes crash at quit, so disabled for now, uncomment for testing
    
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
            self.arrow_grabbed_axis = None
            self.arrow_grabbed = False
            #XXX todo: change these after theres a ent.hide type way

    def LeftMouseDown(self, mouseinfo):
        self.left_button_down = True
        #ent = r.rayCast(mouseinfo.x, mouseinfo.y)
        results = []
        results = r.rayCast(mouseinfo.x, mouseinfo.y)
        ent = None
        
        if results is not None and results[0] != 0:
            id = results[0]
            ent = r.getEntity(id)

        #print "Got entity:", ent
        if ent is not None:
            r.eventhandled = self.EVENTHANDLED
            #print "Entity position is", ent.pos
            #print "Entity id is", ent.id
            #if self.sel is not ent: #XXX wrappers are not reused - there may now be multiple wrappers for same entity
            if self.arrows is not None and ent.id == self.arrows.id:
                u = results[-2]
                v = results[-1]
                #print "ARROW and UV", u, v
                self.arrow_grabbed = True
                if u < 0.421875:
                    #print "arrow is blue / z"
                    self.arrow_grabbed_axis = 2
                elif u < 0.70703125:
                    #print "arrow is green / y"
                    self.arrow_grabbed_axis = 1
                elif u <= 1.0:
                    #print "arrow is red / x"
                    self.arrow_grabbed_axis = 0
                else:
                    print "arrow got screwed..."
                    self.arrow_grabbed_axis = None
                    self.arrow_grabbed = False
                
            if self.sel is None or self.sel.id != ent.id: #a diff ent than prev sel was changed
                self.select(ent)
        
        else:
            self.sel = None
            self.widget.label.text = "<none>"
            self.hideArrows()
            
    def LeftMouseUp(self, mouseinfo):
        self.left_button_down = False
        if self.sel:
            r.networkUpdate(self.sel.id)
        
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
        """dragging objects around - now free movement based on view,
        dragging different axis etc in the manipulator to be added."""

        if not self.canvas.IsHidden():
            if self.left_button_down :
                if self.sel is not None:
                    if self.arrow_grabbed:
                        pos = [self.sel.pos[0], self.sel.pos[1], self.sel.pos[2]]
                        rightvec = r.getCameraRight()
                        upvec = r.getCameraUp()
                        
                        #print "arrow grabbed by",  self.arrow_grabbed_axisx

                    
                        mov = mouseinfo.rel_x + mouseinfo.rel_y
                        mov /= 7.0
                        
                        if self.arrow_grabbed_axis == 2:
                            pos[self.arrow_grabbed_axis] -= mov
                        else:
                            mov *= rightvec[self.arrow_grabbed_axis]/abs(rightvec[self.arrow_grabbed_axis])
                            pos[self.arrow_grabbed_axis] += mov
                
                        self.sel.pos = pos[0], pos[1], pos[2]
                        self.arrows.pos = pos[0], pos[1], pos[2]

                    else:
                        oldvec = Vector3(self.sel.pos)
                        upvec = Vector3(r.getCameraUp())
                        rightvec = Vector3(r.getCameraRight())
                        #print "MouseMove:", mouseinfo.x, mouseinfo.y, r.getCameraUp(), r.getCameraRight()
                        n = 0.1 #is not doing correct raycasting to plane to see pos, this is a multiplier for the crude movement method here now
                        newvec = oldvec - (upvec * mouseinfo.rel_y * n) + (rightvec * mouseinfo.rel_x * n)
                        
                        self.arrows.pos = newvec.x, newvec.y, newvec.z
                        self.sel.pos = newvec.x, newvec.y, newvec.z
                        #r.networkUpdate(self.sel.id)
                        #XXX also here the immediate network sync is not good,
                        #refactor out from pos setter to a separate network_update() call

    def on_exit(self):
        r.logDebug("EditGUI exiting...")
        
        if r.restart:
            #r.logDebug("...restarting...")
            self.hideArrows()
        
        modu = r.getQtModule()
        if self.canvas is not None:
            modu.DeleteCanvas(self.canvas)

        r.logDebug("   ...exit done.")


    def on_hide(self):
        self.hideArrows()
        self.sel = None
        
#barrel on 0.5 in viila: 
# Upload succesfull. Asset id: 35da6174-8743-4026-a83e-18b23984120d, 
# inventory id: 12c3df2d-ef3b-490e-8615-2f89abb7375d.



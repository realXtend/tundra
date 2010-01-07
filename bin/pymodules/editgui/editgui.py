
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
from PythonQt.QtCore import QFile, QSize
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

#are in Naali QObject QTModule::UICanvas as QEnums, in PythonQt module now
#naali.py: UICanvas = PythontQt.__dict__['QTModule::UICanvas']
#INTERNAL = 1
#EXTERNAL = 0

OIS_KEY_ALT = 256
OIS_KEY_CTRL = 16
OIS_KEY_M = 50
OIS_KEY_S = 31
OIS_KEY_R = 19
OIS_KEY_U = 22
OIS_KEY_D = 32
OIS_KEY_Z = 44
OIS_KEY_ESC = 1
OIS_KEY_DEL = 211

DEV = False #if this is false, the canvas is added to the controlbar

class MeshAssetidEditline(QLineEdit):
    def __init__(self, mainedit, *args):
        self.mainedit = mainedit #to be able to query the selected entity at drop
        QLineEdit.__init__(self, *args)

    def accept(self, ev):
        return ev.mimeData().hasFormat("application/vnd.inventory.item")

    def dragEnterEvent(self, ev):
        if self.accept(ev):
            ev.acceptProposedAction()

    def dragMoveEvent(self, ev):
        if self.accept(ev):
            ev.acceptProposedAction()

    def dropEvent(self, ev):
        print "Got meshid_drop:", self, ev
        if not self.accept(ev):
            return

        mimedata = ev.mimeData()
        invitem = mimedata.data("application/vnd.inventory.item")

        #some_qbytearray_thing = invitem[:4] #struct.unpack('i', 
        data = invitem[4:].decode('utf-16-be') #XXX how it happens to be an win: should be explicitly encoded to latin-1 or preferrably utf-8 in the c++ inventory code
        #print data
        asset_type, inv_id, inv_name, asset_ref = data.split(';')

        ent = self.mainedit.sel #is public so no need for getter, can be changed to a property if needs a getter at some point
        if ent is not None:
            applymesh(ent, asset_ref)
            self.text = inv_name
        else:
            self.text = "(no scene entity selected)"

        ev.acceptProposedAction()

def applymesh(ent, meshuuid):
    ent.mesh = meshuuid
    #r.logDebug("Mesh asset UUID after before sending to server: %s" % ent.mesh)
    r.sendRexPrimData(ent.id)
    r.logDebug("Mesh asset UUID after prim data sent to server: %s" % ent.mesh)

class EditGUI(Component):
    EVENTHANDLED = False
    UIFILE = "pymodules/editgui/editobject.ui"
    
    MANIPULATE_NONE = 0
    MANIPULATE_MOVE = 1
    MANIPULATE_ROTATE = 2
    MANIPULATE_SCALE = 3
    
    AXIS_X = 0
    AXIS_Y = 1
    AXIS_Z = 2
    
    UPDATE_INTERVAL = 0.05 #how often the networkUpdate will be sent
    
    def __init__(self):
        Component.__init__(self)
        loader = QUiLoader()
        #mode = INTERNAL
        #if DEV:
        #    mode = EXTERNAL
        #self.canvas = r.createCanvas(mode) #old way before qt refactor
         
        self.move_arrows = None
        uifile = QFile(self.UIFILE)

        ui = loader.load(uifile)
        width = ui.size.width()
        height = ui.size.height()
        
        if not DEV:
            uism = r.getUiSceneManager()
            uiprops = r.createUiWidgetProperty()
            uiprops.widget_name_ = "World Edit"
            uiprops.my_size_ = QSize(width, height)
            #self.proxywidget = uism.AddWidgetToCurrentScene(ui, uiprops)
            self.proxywidget = r.createUiProxyWidget(ui, uiprops)
            #print widget, dir(widget)
            if not uism.AddProxyWidget(self.proxywidget):
                print "Adding the ProxyWidget to the bar failed."
            #modu.AddCanvasToControlBar(ui, "World Edit")
            #XXX change to te new signal. self.proxywidget.connect('Hidden()', self.on_hide)
            self.proxywidget.connect('Visible(bool)', self.on_hide)
        else:
            ui.show()
        
        #self.canvas.SetSize(width, height)
        #self.canvas.SetPosition(30, 30)
        #self.canvas.SetResizable(False)

        #ui.resize(width, height)

        #self.canvas.AddWidget(ui)
        self.widget = ui.MainFrame
        self.widget.label.text = "<none>"

        box = self.widget.findChild("QVBoxLayout")
        if box is not None:
            line = MeshAssetidEditline(self) 
            box.addWidget(line)
            
        self.propedit = r.getPropertyEditor()
        #print pe, pe.setObject, pe.show
        #self.propedit.show()
        
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
        #self.widget.setMesh.connect('clicked()', self.setMesh)
        self.widget.duplicate.connect('clicked()', self.duplicate)
        self.widget.undo.connect('clicked()', self.undo)
        self.widget.redo.connect('clicked()', self.redo)
        self.widget.move_button.connect('clicked()', self.manipulator_move)
        self.widget.scale_button.connect('clicked()', self.manipulator_scale)
        self.widget.rotate_button.connect('clicked()', self.manipulator_rotate)
        
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
        
        self.sel_activated = False #to prevent the selection to be moved on the intial click
        
        self.manipulator_state = self.MANIPULATE_NONE
        
        self.prev_mouse_abs_x = 0
        self.prev_mouse_abs_y = 0

        self.selection_box = None
    
        self.worldstream = r.getServerConnection()
        
        self.dragging = False
        
        #~ self.modified = False
        #~ self.modifying = False
        self.time = 0
        
        self.keypressed = False
        
        self.shortcuts = {
            (OIS_KEY_ESC, 0): self.deselect,
            (OIS_KEY_M, OIS_KEY_ALT): self.manipulator_move,#"ALT+M", #move
            (OIS_KEY_S, OIS_KEY_ALT): self.manipulator_scale,#"ALT+S" #, #scale
            (OIS_KEY_DEL, 0): self.deleteObject,
            (OIS_KEY_Z, OIS_KEY_CTRL): self.undo, 
            (OIS_KEY_D, OIS_KEY_ALT): self.duplicate, 
            #(OIS_KEY_R, ALT): self.manipulator_rotate #rotate
        }
        
        self.active = False
        
        self.canmove = False
    
    def manipulator_move(self):
        if self.keypressed:
            self.keypressed = False
            if not self.widget.move_button.isChecked():
                #print "not activated"
                #print self.widget.move_button, dir(self.widget.move_button)
                self.widget.move_button.setChecked(True)
            else:
                #print "activated"
                self.widget.move_button.setChecked(False)

        if not self.widget.move_button.isChecked():
            self.hideArrows()
        else: #activated
            if self.sel is not None:
                self.drawMoveArrows(self.sel)
                self.manipulator_state = self.MANIPULATE_MOVE
                self.widget.scale_button.setChecked(False)
                self.widget.rotate_button.setChecked(False)
            else:
                self.manipulator_state = self.MANIPULATE_NONE
                self.widget.move_button.setChecked(False)
        
    def manipulator_scale(self):
        if self.keypressed:
            self.keypressed = False
            if not self.widget.scale_button.isChecked():
                #print "not activated"
                #print self.widget.move_button, dir(self.widget.move_button)
                self.widget.scale_button.setChecked(True)
            else:
                #print "activated"
                self.widget.scale_button.setChecked(False)

        if not self.widget.scale_button.isChecked():
            self.hideArrows()
        else: #activated
            #self.widget.move_button.toggle()
            if self.sel is not None:
                self.manipulator_state = self.MANIPULATE_SCALE
                self.drawMoveArrows(self.sel)
                self.widget.move_button.setChecked(False)
                self.widget.rotate_button.setChecked(False)
            else:
                self.widget.scale_button.setChecked(False)
                self.manipulator_state = self.MANIPULATE_NONE

    def manipulator_rotate(self):
        if self.keypressed:
            self.keypressed = False
            if not self.widget.rotate_button.isChecked():
                #print "not activated"
                #print self.widget.move_button, dir(self.widget.move_button)
                self.widget.rotate_button.setChecked(True)
            else:
                #print "activated"
                self.widget.rotate_button.setChecked(False)

        if not self.widget.rotate_button.isChecked():
            self.hideArrows()
        else: #activated
            if self.sel is not None:
                self.hideArrows() #remove this when we have the arrows for rotating
                self.manipulator_state = self.MANIPULATE_ROTATE
                self.widget.scale_button.setChecked(False)
                self.widget.move_button.setChecked(False)
            else:
                self.widget.rotate_button.setChecked(False) 
                self.manipulator_state = self.MANIPULATE_NONE
    
    def float_equal(self, a,b):
        #print abs(a-b), abs(a-b)<0.01
        if abs(a-b)<0.01:
            return True
        else:
            return False

    def changepos(self, i, v):
        #XXX NOTE / API TODO: exceptions in qt slots (like this) are now eaten silently
        #.. apparently they get shown upon viewer exit. must add some qt exc thing somewhere
        #print "pos index %i changed to: %f" % (i, v)
        ent = self.sel
        
        if ent is not None:
            #print "sel pos:", ent.pos, pos[i], v
            pos = list(ent.pos) #should probably wrap Vector3, see test_move.py for refactoring notes. 
    
            if not self.float_equal(pos[i],v):
                pos[i] = v
                #converted to list to have it mutable
                ent.pos = pos[0], pos[1], pos[2] #XXX API should accept a list/tuple too .. or perhaps a vector type will help here too
                #print "=>", ent.pos
                self.move_arrows.pos = pos[0], pos[1], pos[2]
                #self.selection_box.pos = pos[0], pos[1], pos[2]

                self.widget.xpos.setValue(pos[0])
                self.widget.ypos.setValue(pos[1])
                self.widget.zpos.setValue(pos[2])
                self.modified = True
                if not self.dragging:
                    r.networkUpdate(ent.id)
            
    def changescale(self, i, v):
        ent = self.sel
        if ent is not None:
            oldscale = list(ent.scale)
            scale = list(ent.scale)
                
            if not self.float_equal(scale[i],v):
                scale[i] = v
                if self.widget.scale_lock.checked:
                    diff = scale[i] - oldscale[i]
                    for index in range(len(scale)):
                        #print index, scale[index], index == i
                        if index != i:
                            scale[index] += diff
                
                ent.scale = scale[0], scale[1], scale[2]
                
                if not self.dragging:
                    r.networkUpdate(ent.id)
                
                self.widget.scalex.setValue(scale[0])
                self.widget.scaley.setValue(scale[1])
                self.widget.scalez.setValue(scale[2])
                self.modified = True

                self.update_selection()
            
    def changerot(self, i, v):
        #XXX NOTE / API TODO: exceptions in qt slots (like this) are now eaten silently
        #.. apparently they get shown upon viewer exit. must add some qt exc thing somewhere
        #print "pos index %i changed to: %f" % (i, v)
        ent = self.sel
        if ent is not None:
            #print "sel orientation:", ent.orientation
            #from euler x,y,z to to quat
            euler = list(quat_to_euler(ent.orientation))
                
            if not self.float_equal(euler[i],v):
                euler[i] = v
                ort = euler_to_quat(euler)
                #print euler, ort
                #print euler, ort
                ent.orientation = ort
                if not self.dragging:
                    r.networkUpdate(ent.id)
                    
                self.modified = True

                self.move_arrows.orientation = ort
                self.selection_box.orientation = ort
            
    def itemActivated(self, item=None): #the item from signal is not used, same impl used by click
        #print "Got the following item index...", item, dir(item), item.data, dir(item.data) #we has index, now what? WIP
        current = self.widget.treeWidget.currentItem()
        text = current.text(0)
        #print "Selected:", text
        if self.widgetList.has_key(text):
            self.select(self.widgetList[text][0])
    
    def undo(self):
        #print "undo clicked"
        ent = self.sel
        if ent is not None:
            #print ent.uuid
            #worldstream = r.getServerConnection()
            self.worldstream.SendObjectUndoPacket(ent.uuid)
            #self.hideArrows()
            #self.sel = None
            self.update_guivals()
            self.modified = False

    def redo(self):
        #print "redo clicked"
        ent = self.sel
        if ent is not None:
            #print ent.uuid
            #worldstream = r.getServerConnection()
            self.worldstream.SendObjectRedoPacket(ent.uuid)
            #self.hideArrows()
            #self.sel = None
            self.update_guivals()
            self.modified = False
            
    def duplicate(self):
        #print "duplicate clicked"
        ent = self.sel
        if ent is not None:
            #print e, e.uuid, e.editable, e.updateflags
            self.worldstream.SendObjectDuplicatePacket(ent.id, ent.updateflags, 1, 1, 1) #nasty hardcoded offset
        
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
        ent = self.sel
        if ent is not None:
            #worldstream = r.getServerConnection()
            #print worldstream, dir(worldstream), worldstream.SendObjectDeRezPacket
            self.worldstream.SendObjectDeRezPacket(ent.id, r.getTrashFolderId())
            self.hideArrows()
            self.hideSelector()
            id, tWid = self.widgetList.pop(str(ent.id))
            #print tWid, tWid.text(0)
            tWid.delete()
            self.deselect()
            self.sel = None

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
        if self.move_arrows.id == ent.id:
            arrows = True

        if ent.id != 0 and ent.id > 10 and ent.id != r.getUserAvatarId() and not arrows: #terrain seems to be 3 and scene objects always big numbers, so > 10 should be good
            self.sel = ent
            self.sel_activated = False
            self.worldstream.SendObjectSelectPacket(ent.id)
            
            if not self.widgetList.has_key(str(self.sel.id)):
                tWid = QTreeWidgetItem(self.widget.treeWidget)
                id = self.sel.id
                tWid.setText(0, id)
                
                self.widgetList[str(id)] = (ent, tWid)

            #print "Selected entity:", self.sel.id, "at", self.sel.pos#, self.sel.name

            #update the gui vals to show what the newly selected entity has
            self.update_guivals()
            self.widget.label.text = ent.id
            
            self.update_selection()
            
            qprim = r.getQPrim(ent.id)
            self.propedit.setObject(qprim)
            self.propedit.show()
           #print "Set propedit object to:", qprim, dir(qprim)
    
    def deselect(self):
        if self.sel is not None:
            self.sel = None
            self.widget.label.text = "<none>"
            self.hideArrows()
            self.hideSelector()
            self.prev_mouse_abs_x = 0
            self.prev_mouse_abs_y = 0
            self.canmove = False
            self.arrow_grabbed_axis = None
            self.arrow_grabbed = False
            self.propedit.hide()
        
    def update_selection(self):             
        bb = list(self.sel.boundingbox)
        scale = list(self.sel.scale)
        min = Vector3(bb[0], bb[1], bb[2])
        max = Vector3(bb[3], bb[4], bb[5])
        height = abs(bb[4] - bb[1]) 
        width = abs(bb[3] - bb[0])
        depth = abs(bb[5] - bb[2])

        if bb[6] == 0: #0 means CustomObject
            height += scale[0]#*1.2
            width += scale[1] #*1.2
            depth += scale[2]#*1.2

            self.selection_box.pos = self.sel.pos
            
            self.selection_box.scale = height, width, depth#depth, width, height
            self.selection_box.orientation = self.sel.orientation
        else:
            r.logDebug("EditGUI: EC_OgreMesh clicked...")

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
         
        self.selection_box.pos = self.sel.pos
        self.selection_box.orientation =self.sel.orientation
        
            #~ if self.cam is None and ogreroot:
                #~ rs = root.getRenderSystem()
                #~ vp = rs._getViewport()
                #~ self.cam = vp.getCamera()
                #~ self.drawMoveArrows(ent)
            #~ elif self.cam is not None and ogreroot: 
                #~ self.drawMoveArrows(ent)
            #~ else:
    
    def drawMoveArrows(self, ent):
        #print "drawMoveArrows", self.move_arrows
        x, y, z = ent.pos
        pos = x, y, z
        
        sx, sy, sz = ent.scale
        scale = sx, sy, sz
        
        ox, oy, oz, ow = ent.orientation
        ort = ox, oy, oz, ow
        
        self.showArrow(pos, scale, ort)
        
    def createArrows(self):
        #print "\nCreating arrows!\n"
        ent = r.createEntity("axes.mesh", 606847240)
        return ent
        
    def showArrow(self, pos, scale, ort):
        #print "Showing arrows!"
        #if self.move_arrows is not None:
        self.move_arrows.pos = pos
        self.move_arrows.scale = 0.2, 0.2, 0.2
        #self.arrow.setOrientation(self.cam.DerivedOrientation)
        self.move_arrows.orientation = ort
  
    def hideArrows(self):
        #print "Hiding arrows!"
        if self.move_arrows is not None:
            #pos = self.move_arrows.pos #very ugly hack... seems to correct the annoying error with grabbing an object the first time
            #scale = self.move_arrows.scale
            #print self.move_arrows.scale, self.move_arrows.pos
            self.move_arrows.scale = 0.0, 0.0, 0.0 #ugly hack
            self.move_arrows.pos = 0.0, 0.0, 0.0 #another ugly hack
        
        self.arrow_grabbed_axis = None
        self.arrow_grabbed = False
        self.manipulator_state = self.MANIPULATE_NONE
        
        self.widget.move_button.setChecked(False)
        self.widget.rotate_button.setChecked(False)
        self.widget.scale_button.setChecked(False)
        #XXX todo: change these after theres a ent.hide type way
        
    def hideSelector(self):
        if self.selection_box is not None:
            self.selection_box.scale = 0.0, 0.0, 0.0
            self.selection_box.pos = 0.0, 0.0, 0.0
            
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
            width, height = r.getScreenSize()
            normalized_width = 1/width
            normalized_height = 1/height
            mouse_abs_x = normalized_width * mouseinfo.x
            mouse_abs_y = normalized_height * mouseinfo.y
            self.prev_mouse_abs_x = mouse_abs_x
            self.prev_mouse_abs_y = mouse_abs_y

            r.eventhandled = self.EVENTHANDLED
            #print "Entity position is", ent.pos
            #print "Entity id is", ent.id
            #if self.sel is not ent: #XXX wrappers are not reused - there may now be multiple wrappers for same entity
            if self.selection_box is None:
                self.selection_box = r.createEntity("Selection.mesh", 0)
            
            if self.move_arrows is None:
                self.move_arrows = self.createArrows()
                self.hideArrows()
            
            if ent.id == self.move_arrows.id:
                u = results[-2]
                v = results[-1]
                #print "ARROW and UV", u, v
                self.arrow_grabbed = True
                if u < 0.421875:
                    #print "arrow is blue / z"
                    self.arrow_grabbed_axis = self.AXIS_Z
                elif u < 0.70703125:
                    #print "arrow is green / y"
                    self.arrow_grabbed_axis = self.AXIS_Y
                elif u <= 1.0:
                    #print "arrow is red / x"
                    self.arrow_grabbed_axis = self.AXIS_X
                else:
                    print "arrow got screwed..."
                    self.arrow_grabbed_axis = None
                    self.arrow_grabbed = False
                
            if self.sel is None or self.sel.id != ent.id: #a diff ent than prev sel was changed
                self.select(ent)
                self.canmove = True
            elif self.sel.id == ent.id:
                self.canmove = True
        
        else:
            #print "canmove:", self.canmove
            self.canmove = False
            #~ self.sel = None
            #~ self.widget.label.text = "<none>"
            #~ self.hideArrows()
            #~ self.hideSelector()
            #~ self.prev_mouse_abs_x = 0
            #~ self.prev_mouse_abs_y = 0
            
    def LeftMouseUp(self, mouseinfo):
        self.left_button_down = False
        
        if self.sel:
            if self.sel_activated and self.dragging:
                #print "LeftMouseUp, networkUpdate call"
                r.networkUpdate(self.sel.id)
            
            self.sel_activated = True
        
        if self.dragging:
            self.dragging = False
            
        self.arrow_grabbed_axis = None
        self.arrow_grabbed = False
        
    def RightMouseDown(self, mouseinfo):
        self.right_button_down = True
        
    def RightMouseUp(self, mouseinfo):
        self.right_button_down = False
        
    def on_mouseclick(self, click_id, mouseinfo, callback):
        #print "MouseMove", mouseinfo.x, mouseinfo.y, self.canvas.IsHidden()
        if self.active: #XXXnot self.canvas.IsHidden():
            #print "on_mouseclick", click_id, mouseinfo.x, mouseinfo.y
            #print "Point!"
            if self.mouse_events.has_key(click_id):
                self.mouse_events[click_id](mouseinfo)
                #print "on_mouseclick", click_id, self.mouse_events[click_id]
            #else:
                #print "unknown click_id?", self.mouse_events
            
    def on_mousemove(self, mouseinfo, callback):
        """dragging objects around - now free movement based on view,
        dragging different axis etc in the manipulator to be added."""

        if self.active: #XXX not self.canvas.IsHidden():
            if self.left_button_down :
                #print "on_mousemove + hold:", mouseinfo
                if self.sel is not None and self.sel_activated and self.canmove:
                    self.dragging = True              
                    fov = r.getCameraFOV()
                    rightvec = Vector3(r.getCameraRight())
                    upvec = Vector3(r.getCameraUp())
                    campos = Vector3(r.getCameraPosition())
                    entpos = Vector3(self.sel.pos)
                    width, height = r.getScreenSize()
                    
                    normalized_width = 1/width
                    normalized_height = 1/height
                    mouse_abs_x = normalized_width * mouseinfo.x
                    mouse_abs_y = normalized_height * mouseinfo.y
                    
                    length = (campos-entpos).length
                    worldwidth = (math.tan(fov/2)*length) * 2
                    worldheight = (height*worldwidth) / width
                    movedx = mouse_abs_x - self.prev_mouse_abs_x
                    movedy = mouse_abs_y - self.prev_mouse_abs_y
                    #used in freemoving to get the size of movement right
                    amount_x = (worldwidth * movedx)
                    amount_y = (worldheight * movedy)
                    
                    if self.arrow_grabbed:
                        #rightvec = r.getCameraRight()
                        #upvec = r.getCameraUp()
                        #mov = mouseinfo.rel_x + mouseinfo.rel_y
                        
                        temp = [0,0,0]
                        temp[self.arrow_grabbed_axis] = 1
                        axis_vec = Vector3(temp)
                        #print amount_x, amount_y
                        mousey_on_arrow_projection = upvec.dot(axis_vec) * axis_vec
                        lengthy = mousey_on_arrow_projection.length * amount_y
                        mousex_on_arrow_projection = rightvec.dot(axis_vec) * axis_vec
                        lengthx = mousex_on_arrow_projection.length * amount_x
                        #print "X, Y", lengthx, lengthy
                        #print mouse_on_arrow_projection, upvec, axis_vec
                        
                        if self.manipulator_state == self.MANIPULATE_MOVE: #arrow move
                            pos = list(self.sel.pos)
                            
                            #mov /= 7.0
                            #print rightvec[self.arrow_grabbed_axis], rightvec
                            if self.arrow_grabbed_axis == self.AXIS_Z:
                                mov = lengthy
                                #print mov, pos[self.arrow_grabbed_axis],
                                pos[self.arrow_grabbed_axis] -= mov
                                #print pos[self.arrow_grabbed_axis]
                            else:
                                mov = lengthx 
                                mov *= rightvec[self.arrow_grabbed_axis]/abs(rightvec[self.arrow_grabbed_axis])
                                #print mov, pos[self.arrow_grabbed_axis],
                                pos[self.arrow_grabbed_axis] += mov
                            
                            #print pos[self.arrow_grabbed_axis]
                    
                            self.sel.pos = pos[0], pos[1], pos[2]
                            self.move_arrows.pos = pos[0], pos[1], pos[2]
                            #self.update_guivals()
                                
                        elif self.manipulator_state == self.MANIPULATE_SCALE: #arrow scaling
                            #print "should change scale!"
                            scale = list(self.sel.scale)

                            #mov /= 9.0
                            
                            if self.arrow_grabbed_axis == self.AXIS_Z:
                                mov = lengthy
                                scale[self.arrow_grabbed_axis] -= mov
                            else:
                                #mov *= rightvec[self.arrow_grabbed_axis]/abs(rightvec[self.arrow_grabbed_axis])
                                mov = lengthx
                                mov *= rightvec[self.arrow_grabbed_axis]/abs(rightvec[self.arrow_grabbed_axis])
                                scale[self.arrow_grabbed_axis] += mov
                    
                            self.sel.scale = scale[0], scale[1],scale[2]
                            #self.update_guivals()
                            self.update_selection()

                    else: #freemove
                        #~ oldvec = Vector3(self.sel.pos)
                        #~ upvec = Vector3(r.getCameraUp())
                        #~ rightvec = Vector3(r.getCameraRight())
                        #~ #print "MouseMove:", mouseinfo.x, mouseinfo.y, r.getCameraUp(), r.getCameraRight()
                        #~ n = 0.1 #is not doing correct raycasting to plane to see pos, this is a multiplier for the crude movement method here now
                        #~ newvec = oldvec - (upvec * mouseinfo.rel_y * n) + (rightvec * mouseinfo.rel_x * n)
                        
                        #~ self.move_arrows.pos = newvec.x, newvec.y, newvec.z
                        #~ self.sel.pos = newvec.x, newvec.y, newvec.z
                        #~ #r.networkUpdate(self.sel.id)
                        #~ #XXX also here the immediate network sync is not good,
                        #~ #refactor out from pos setter to a separate network_update() call

                        newpos = entpos + (amount_x * rightvec) - (amount_y * upvec)
                        
                        #print newpos
                        self.sel.pos = newpos.x, newpos.y, newpos.z
                        #print worldwidth
                        
                    self.prev_mouse_abs_x = mouse_abs_x
                    self.prev_mouse_abs_y = mouse_abs_y
                    
                    self.update_guivals()
   
    def on_keyup(self, keycode, keymod, callback):
        if self.active:
            #print keycode, keymod
            if self.shortcuts.has_key((keycode, keymod)):
                self.keypressed = True
                self.shortcuts[(keycode, keymod)]()
                callback(True)
        
    def on_inboundnetwork(self, evid, name, callback):
        return False
        #print "editgui got an inbound network event:", id, name

    def on_exit(self):
        r.logDebug("EditGUI exiting...")
        
        if r.restart:
            r.logDebug("...restarting...")
            self.hideArrows()
        #~ else:
            #~ r.logDebug("...not restarting...")
        
        self.proxywidget.hide() #prevent a crash at exit
        
        if 0: #XXX self.canvas is not None:
            modu = r.getQtModule()
            modu.DeleteCanvas(self.canvas)

        #r.logDebug("   ...exit done.")

    def on_hide(self, shown):
        self.active = shown
        
        if self.active:
            self.sel = None
            try:
                if self.move_arrows is not None:
                    ent = self.move_arrows.id 
            #is called by qt also when viewer is exiting,
            #when the scene (in rexlogic module) is not there anymore.
            except RuntimeError, e:
                r.logDebug("on_hide: scene not found")
            else:
                self.deselect()
        else:
            self.deselect()
            
    def update(self, time):
        #print "here", time
        if self.active: #XXX not self.canvas.IsHidden(): #do we need this here?
            self.time += time
            ent = self.sel
            if self.time > self.UPDATE_INTERVAL:
                if ent is not None:
                    try:
                        sel_pos = self.selection_box.pos
                        arr_pos = self.move_arrows.pos
                        ent_pos = ent.pos
                        #print ent_pos, sel_pos
                        if sel_pos != ent_pos:
                            #print "oh dear"
                            self.time = 0
                            self.selection_box.pos = ent_pos
                        if arr_pos != ent_pos:
                            self.move_arrows.pos = ent_pos
                    except RuntimeError, e:
                        r.logDebug("update: scene not found")
                        
                    #~ self.time += time
                    #~ if self.time >= self.UPDATE_INTERVAL:
                        #~ #print "hep"
                        #~ self.time = 0
                        #~ ent = self.sel
                        #~ if self.modified and ent is not None:
                            #~ r.networkUpdate(ent.id)
                            #~ self.modified = False
                    
#barrel on 0.5 in viila: 
# Upload succesfull. Asset id: 35da6174-8743-4026-a83e-18b23984120d, 
# inventory id: 12c3df2d-ef3b-490e-8615-2f89abb7375d.


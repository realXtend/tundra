
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
from circuits import Component
from PythonQt.QtUiTools import QUiLoader
from PythonQt.QtCore import QFile
from vector3 import Vector3 #for view based editing calcs now that Vector3 not exposed from internals
from conversions import quat_to_euler, euler_to_quat #for euler - quat -euler conversions

try:
    window
    manipulator 
except: #first run
    try:
        import window
        import manipulator
    except ImportError, e:
        print "couldn't load window and manipulator:", e
else:
    window = reload(window)
    manipulator = reload(manipulator)

#NOTE: these are not ported yet after using OIS was dropped, so don't work
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

 
class ObjectEdit(Component):
    EVENTHANDLED = False
 
    UPDATE_INTERVAL = 0.05 #how often the networkUpdate will be sent
    
    MANIPULATE_FREEMOVE = 0
    MANIPULATE_MOVE = 1
    MANIPULATE_SCALE = 2
    MANIPULATE_ROTATE = 3
    
    SELECTIONRECT = "pymodules/objectedit/selection.ui"

    def __init__(self):
        self.sels = []  
        Component.__init__(self)
        self.window = window.ObjectEditWindow(self)
        self.resetValues()
        self.worldstream = r.getServerConnection()
        
        self.mouse_events = {
            #r.LeftMouseClickPressed: self.LeftMousePressed,
            r.InWorldClick: self.LeftMousePressed,
            r.LeftMouseClickReleased: self.LeftMouseReleased,  
            r.RightMouseClickPressed: self.RightMousePressed,
            r.RightMouseClickReleased: self.RightMouseReleased
        }

        self.shortcuts = {
            #r.PyObjectEditDeselect: self.deselect,
            r.PyObjectEditToggleMove: self.window.manipulator_move,#"ALT+M", #move
            r.PyObjectEditToggleScale: self.window.manipulator_scale,#"ALT+S" #, #scale
            r.Delete: self.deleteObject,
            r.Undo: self.undo, 
            r.PyDuplicateDrag: self.duplicateStart, 
        }
        
        self.resetManipulators()
        
        loader = QUiLoader()
        selectionfile = QFile(self.SELECTIONRECT)
        self.selection_rect = loader.load(selectionfile)
        rectprops = r.createUiWidgetProperty(2)
        #uiprops.widget_name_ = "Selection Rect"
        
        #uiprops.my_size_ = QSize(width, height) #not needed anymore, uimodule reads it
        proxy = r.createUiProxyWidget(self.selection_rect, rectprops)
        uism = r.getUiSceneManager()
        uism.AddProxyWidget(proxy)
        proxy.setWindowFlags(0) #changing it to Qt::Widget
        
        self.selection_rect.setGeometry(0,0,0,0)
        self.selection_rect_startpos = None
        
        r.c = self #this is for using objectedit from command.py
        
    def resetValues(self):
        self.left_button_down = False
        self.right_button_down = False
        self.sel_activated = False #to prevent the selection to be moved on the intial click
        self.prev_mouse_abs_x = 0
        self.prev_mouse_abs_y = 0
        self.dragging = False
        self.time = 0
        self.keypressed = False
        self.windowActive = False
        self.canmove = False
        self.selection_box = None
        self.selection_rect_startpos = None
    
    def resetManipulators(self):
        self.manipulatorsInit = False
        self.manipulators = {}
        self.manipulators[self.MANIPULATE_MOVE] =  manipulator.MoveManipulator(self)
        self.manipulators[self.MANIPULATE_SCALE] =  manipulator.ScaleManipulator(self)
        self.manipulators[self.MANIPULATE_FREEMOVE] =  manipulator.FreeMoveManipulator(self)
        #self.manipulators[self.MANIPULATE_ROTATE] =  manipulator.RotateManipulator(self)
        self.manipulator = self.manipulators[self.MANIPULATE_FREEMOVE]
 
    def baseselect(self, ent):
        ent, children = self.parentalCheck(ent)
        
        self.sel_activated = False
        self.worldstream.SendObjectSelectPacket(ent.id)
        self.updateSelectionBox()
        self.changeManipulator(self.MANIPULATE_FREEMOVE)
        
        return ent, children
        
    def parentalCheck(self, ent):
        exitLoop = False
        parent = False
        children = []
        
        while(not exitLoop):
            qprim = r.getQPrim(ent.id)
            if qprim is not None:
                if qprim.ParentId != 0:
                    #~ r.logInfo("Entity had a parent, lets pick that instead!")
                    ent = r.getEntity(qprim.ParentId)
                else:
                    #~ r.logInfo("Entity had no parent, maybe it has children?")
                    children = qprim.GetChildren()
                    if len(children)>0:
                        parent = True
                        
                    exitLoop = True
                    
        return ent, children
        
    def select(self, ent):
        self.sels = []
        ent, children = self.baseselect(ent)
        self.sels.append(ent)
        self.window.selected(ent, False) 
        self.canmove = True
        
        self.highlightChildren(children)

    def multiselect(self, ent):
        self.sels.append(ent)
        ent, children = self.baseselect(ent)
        self.window.selected(ent, True) 
        
        self.highlightChildren(children)
    
    def highlightChildren(self, children):
        for child_id in children:
            child = r.getEntity(int(child_id))
            self.window.addToList(child)
            self.window.highlightEntityFromList(child)
            
    def deselect(self):
        if len(self.sels)>0:
            #XXX might need something here?!
            self.sels = []
            self.hideSelector()
            
            self.hideManipulator() #manipulator

            self.prev_mouse_abs_x = 0
            self.prev_mouse_abs_y = 0
            self.canmove = False

            self.window.deselected()

    def updateSelectionBox(self): 
        ent = self.active #XXX use first or last... ?
        if ent is not None:
            bb = list(ent.boundingbox)
            scale = list(ent.scale)
            min = Vector3(bb[0], bb[1], bb[2])
            max = Vector3(bb[3], bb[4], bb[5])
            height = abs(bb[4] - bb[1]) 
            width = abs(bb[3] - bb[0])
            depth = abs(bb[5] - bb[2])

            if 1:#bb[6] == 0: #0 means CustomObject
                height += scale[0]#*1.2
                width += scale[1] #*1.2
                depth += scale[2]#*1.2

                self.selection_box.pos = ent.pos
                
                self.selection_box.scale = height, width, depth#depth, width, height
                self.selection_box.orientation = ent.orientation
            else:
                r.logDebug("EditGUI: EC_OgreMesh clicked...")

    def changeManipulator(self, id):
        #r.logInfo("changing manipulator to " + str(id))
        
        newmanipu = self.manipulators[id]
        if newmanipu.NAME != self.manipulator.NAME:
            #r.logInfo("was something completely different")
            self.manipulator.hideManipulator()
            self.manipulator = newmanipu
        
        #ent = self.active
        self.manipulator.showManipulator(self.sels)
    
    def hideManipulator(self):
        self.manipulator.hideManipulator()

    def hideSelector(self):
        try: #XXX! without this try-except, if something is selected, the viewer will crash on exit
            if self.selection_box is not None:
                self.selection_box.scale = 0.0, 0.0, 0.0
                self.selection_box.pos = 0.0, 0.0, 0.0
        except RuntimeError, e:
            r.logDebug("hideSelector failed")
            
    def LeftMousePressed(self, mouseinfo):
        r.logDebug("LeftMousePressed") #, mouseinfo, mouseinfo.x, mouseinfo.y
        r.logDebug("point " + str(mouseinfo.x) + "," + str(mouseinfo.y))
        self.dragStarted(mouseinfo)
        
        if self.selection_box is None:
            self.selection_box = r.createEntity("Selection.mesh", 0)
        
        self.left_button_down = True
        
        results = []
        results = r.rayCast(mouseinfo.x, mouseinfo.y)
        ent = None
        
        if results is not None and results[0] != 0:
            id = results[0]
            ent = r.getEntity(id)

        if not self.manipulatorsInit:
            self.manipulatorsInit = True
            for manipulator in self.manipulators.values():
                manipulator.initVisuals()
            
        self.manipulator.initManipulation(ent, results)

        #print "Got entity:", ent
        if ent is not None:
            if not self.manipulator.compareIds(ent.id) and ent.id != self.selection_box.id:                
                #if self.sel is not ent: #XXX wrappers are not reused - there may now be multiple wrappers for same entity
                
                r.eventhandled = self.EVENTHANDLED
                found = False
                for entity in self.sels:
                    if entity.id == ent.id:
                        found = True
               
                if self.active is None or self.active.id != ent.id: #a diff ent than prev sel was changed  
                    if self.validId(ent.id):
                        if not found:
                            self.select(ent)

                elif self.active.id == ent.id: #canmove is the check for click and then another click for moving, aka. select first, then start to manipulate
                    self.canmove = True
                    
        else:
            self.selection_rect_startpos = (mouseinfo.x, mouseinfo.y)
            #print "canmove:", self.canmove
            self.canmove = False
            self.deselect()
            
    def dragStarted(self, mouseinfo):
        width, height = r.getScreenSize()
        normalized_width = 1/width
        normalized_height = 1/height
        mouse_abs_x = normalized_width * mouseinfo.x
        mouse_abs_y = normalized_height * mouseinfo.y
        self.prev_mouse_abs_x = mouse_abs_x
        self.prev_mouse_abs_y = mouse_abs_y

    def LeftMouseReleased(self, mouseinfo):
        self.left_button_down = False
        if self.active: #XXX something here?
            if self.sel_activated and self.dragging:
                for ent in self.sels:
                    #~ print "LeftMouseReleased, networkUpdate call"
                    parent, children = self.parentalCheck(ent)
                    r.networkUpdate(ent.id)
                    for child in children:
                        child_id = int(child)
                        r.networkUpdate(child_id)
            
            self.sel_activated = True
        
        if self.dragging:
            self.dragging = False
            
        self.manipulator.stopManipulating()
        self.duplicateDragStart = False #XXXchange?
        
        if self.selection_rect_startpos is not None:
            self.selection_rect.hide()

            rectx, recty, rectwidth, rectheight = self.selectionRectDimensions(mouseinfo)
            if rectwidth != 0 and rectheight != 0:
                r.logInfo("The selection rect was at: (" +str(rectx) + ", " +str(recty) + ") and size was: (" +str(rectwidth) +", "+str(rectheight)+")") 
                self.selection_rect.setGeometry(0,0,0,0)
            
            self.selection_rect_startpos = None
    
    def selectionRectDimensions(self, mouseinfo):
        rectx = self.selection_rect_startpos[0]
        recty = self.selection_rect_startpos[1]
        
        rectwidth = (mouseinfo.x - rectx)
        rectheight = (mouseinfo.y - recty)
        
        if rectwidth < 0:
            rectx += rectwidth
            rectwidth *= -1
            
        if rectheight < 0:
            recty += rectheight
            rectheight *= -1
            
        return rectx, recty, rectwidth, rectheight
        
    def RightMousePressed(self, mouseinfo):
        #r.logInfo("rightmouse down")
        if self.windowActive:
            self.right_button_down = True
            
            results = []
            results = r.rayCast(mouseinfo.x, mouseinfo.y)
            
            ent = None
            
            if results is not None and results[0] != 0:
                id = results[0]
                ent = r.getEntity(id)
                
            found = False
            #print "Got entity:", ent
            if ent is not None:                
                for entity in self.sels:
                    if entity.id == ent.id:
                        found = True
                
                if self.active is None or self.active.id != ent.id: #a diff ent than prev sel was changed  
                    if self.validId(ent.id):
                        if not found:
                            self.multiselect(ent)
                        else:
                            for _ent in self.sels:
                                if _ent.id == ent.id:
                                    self.sels.remove(_ent)
                                    self.window.deselectSelection(_ent.id)
                        self.canmove = True
                        
            #r.logInfo(str(self.sels))
    def validId(self, id):
        if id != 0 and id > 50: #terrain seems to be 3 and scene objects always big numbers, so > 50 should be good, though randomly created local entities can get over 50...
            if id != r.getUserAvatarId(): #add other avatar id's check
                if not self.manipulator.compareIds(id) and id != self.selection_box.id:
                    return True
        return False
        
    def RightMouseReleased(self, mouseinfo):
        #r.logInfo("rightmouse up")
        self.right_button_down = False
        
    def on_mouseclick(self, click_id, mouseinfo, callback):
        if self.windowActive: #XXXnot self.canvas.IsHidden():
            if self.mouse_events.has_key(click_id):
                self.mouse_events[click_id](mouseinfo)
                #~ r.logInfo("on_mouseclick %d %s" % (click_id, self.mouse_events[click_id]))
                
    def on_mousemove(self, mouseinfo, callback):
        """dragging objects around - now free movement based on view,
        dragging different axis etc in the manipulator to be added."""
        if self.windowActive:
            
            width, height = r.getScreenSize()
            
            normalized_width = 1/width
            normalized_height = 1/height
            mouse_abs_x = normalized_width * mouseinfo.x
            mouse_abs_y = normalized_height * mouseinfo.y
                                
            movedx = mouse_abs_x - self.prev_mouse_abs_x
            movedy = mouse_abs_y - self.prev_mouse_abs_y
            
            if self.left_button_down:
                if self.selection_rect_startpos is not None:# and self.active is None:
                    rectx, recty, rectwidth, rectheight = self.selectionRectDimensions(mouseinfo)
                    #print rectwidth, rectheight
                    self.selection_rect.setGeometry(rectx, recty, rectwidth, rectheight)
                    self.selection_rect.show() #XXX change?
                else:
                    if self.duplicateDragStart:
                        for ent in self.sels:
                            self.worldstream.SendObjectDuplicatePacket(ent.id, ent.updateflags, 0, 0, 0) #nasty hardcoded offset
                        self.duplicateDragStart = False
                            
                    ent = self.active
                    #print "on_mousemove + hold:", mouseinfo
                    if ent is not None and self.sel_activated and self.canmove:
                        self.dragging = True

                        self.manipulator.manipulate(self.sels, movedx, movedy)
                        
                        self.prev_mouse_abs_x = mouse_abs_x
                        self.prev_mouse_abs_y = mouse_abs_y
                        
                        self.window.update_guivals(ent)
   
    def on_input(self, evid, callback):
        if self.windowActive:
            if evid in self.shortcuts:#self.shortcuts.has_key((keycode, keymod)):
                self.keypressed = True
                self.shortcuts[evid]()
                callback(True)
        
    def on_inboundnetwork(self, evid, name, callback):
        return False
        #print "editgui got an inbound network event:", id, name

    def undo(self):
        #print "undo clicked"
        ent = self.active
        if ent is not None:
            self.worldstream.SendObjectUndoPacket(ent.uuid)
            self.window.update_guivals(ent)
            self.modified = False
            self.deselect()

    #~ def redo(self):
        #~ #print "redo clicked"
        #~ ent = self.sel
        #~ if ent is not None:
            #~ #print ent.uuid
            #~ #worldstream = r.getServerConnection()
            #~ self.worldstream.SendObjectRedoPacket(ent.uuid)
            #~ #self.sel = []
            #~ self.update_guivals()
            #~ self.modified = False
            
    def duplicate(self):
        #print "duplicate clicked"
        #ent = self.active
        #if ent is not None:
        for ent in self.sels:
            self.worldstream.SendObjectDuplicatePacket(ent.id, ent.updateflags, 1, 1, 1) #nasty hardcoded offset
        
    def duplicateStart(self):
        self.duplicateDragStart = True
        
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
        if self.active is not None:
            for ent in self.sels:
                #r.logInfo("deleting " + str(ent.id))
                ent, children = self.parentalCheck(ent)
                #~ for child_id in children:
                    #~ child = r.getEntity(int(child_id))
                    #~ self.window.addToList(child)
                    #~ print "deleting", child
                    #~ self.worldstream.SendObjectDeRezPacket(child.id, r.getTrashFolderId())
                    #~ self.window.objectDeleted(str(child.id))
                if len(children) == 0:
                    self.worldstream.SendObjectDeRezPacket(ent.id, r.getTrashFolderId())
                    self.window.objectDeleted(str(ent.id))
                else:
                    r.logInfo("trying to delete a parent, need to fix this!")
            
            self.manipulator.hideManipulator()
            self.hideSelector()        
            self.deselect()
            self.sels = []
            
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
        ent = self.active
        
        if ent is not None:
            #print "sel pos:", ent.pos, pos[i], v
            pos = list(ent.pos) #should probably wrap Vector3, see test_move.py for refactoring notes. 
    
            if not self.float_equal(pos[i],v):
                pos[i] = v
                #converted to list to have it mutable
                ent.pos = pos[0], pos[1], pos[2] #XXX API should accept a list/tuple too .. or perhaps a vector type will help here too
                #print "=>", ent.pos
                self.manipulator.moveTo(self.sels)
                #self.selection_box.pos = pos[0], pos[1], pos[2]

                #self.window.update_posvals(pos)
                self.modified = True
                if not self.dragging:
                    r.networkUpdate(ent.id)
            
    def changescale(self, i, v):
        ent = self.active
        if ent is not None:
            oldscale = list(ent.scale)
            scale = list(ent.scale)
                
            if not self.float_equal(scale[i],v):
                scale[i] = v
                if self.window.mainTab.scale_lock.checked:
                    #XXX BUG does wrong thing - the idea was to maintain aspect ratio
                    diff = scale[i] - oldscale[i]
                    for index in range(len(scale)):
                        #print index, scale[index], index == i
                        if index != i:
                            scale[index] += diff
                
                ent.scale = scale[0], scale[1], scale[2]
                
                if not self.dragging:
                    r.networkUpdate(ent.id)
                
                #self.window.update_scalevals(scale)
                
                self.modified = True

                self.updateSelectionBox()
            
    def changerot(self, i, v):
        #XXX NOTE / API TODO: exceptions in qt slots (like this) are now eaten silently
        #.. apparently they get shown upon viewer exit. must add some qt exc thing somewhere
        #print "pos index %i changed to: %f" % (i, v)
        ent = self.active
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
                #self.window.update_rotvals(ort)
                self.selection_box.orientation = ort
                
    def updateSelectionBoxPositionAndOrientation(self, ent): #XXX riiiight, rename please!
        self.selection_box.pos = ent.pos
        self.selection_box.orientation = ent.orientation
    
    def getActive(self):
        if len(self.sels) > 0:
            ent = self.sels[-1]
            return ent
        return None
        
    active = property(getActive)
    
    def on_exit(self):
        r.logInfo("Object Edit exiting...")

        self.deselect()
        self.window.on_exit()  

        r.logInfo("         ...exit done.")

    def on_hide(self, shown):
        self.windowActive = shown

        
        if self.windowActive:
            self.sels = []
            
            try:
                self.manipulator.hideManipulator()
                #if self.move_arrows is not None:
                    #ent = self.move_arrows.id 
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
        if self.windowActive:
            self.time += time
            if self.sels:
                ent = self.active
                if self.time > self.UPDATE_INTERVAL:
                    try:
                        sel_pos = self.selection_box.pos
                        arr_pos = self.manipulator.getManipulatorPosition()
                        ent_pos = ent.pos
                        if sel_pos != ent_pos:
                            self.time = 0
                            self.selection_box.pos = ent_pos
                        if arr_pos != ent_pos:
                            self.manipulator.moveTo(self.sels)
                    except RuntimeError, e:
                        r.logDebug("update: scene not found")
   
    def on_logout(self, id):
        r.logInfo("Object Edit resetting due to Logout.")
        self.deselect()
        self.sels = []
        self.selection_box = None
        self.resetValues()
        self.resetManipulators()
        
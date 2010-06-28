
"""
A gui tool for editing.

Originally was a basic proof-of-concept and a test of the Python API:
The qt integration for ui together with the manually wrapped entity-component data API
and the viewer non-qt event system for mouse events thru the py plugin system.

Later has been developed to be an actually usable editing tool, and currently is the only tool for that for Naali.

TODO (most work is in api additions on the c++ side, then simple usage here):
- local & global movement
- (WIP, needs network event refactoring) sync changes from over the net to the gui dialog: listen to scene objectupdates
  (this is needed/nice when someone else is moving the same obj at the same time,
   works correctly in slviewer, i.e. the dialogs there update on net updates)
- hilite the selected object
(- list all the objects to allow selection from there)

"""

import rexviewer as r
from circuits import Component
from PythonQt.QtUiTools import QUiLoader
from PythonQt.QtCore import QFile, Qt
import conversions as conv
reload(conv) # force reload, otherwise conversions is not reloaded on python restart in Naali
from PythonQt.QtGui import QVector3D as Vec
from PythonQt.QtGui import QQuaternion as Quat
from naali import inputcontext

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
        self.usingManipulator = False
        self.useLocalTransform = False
        self.cpp_python_handler = None
        
        self.mouse_events = {
            43 : self.LeftMousePressed,
            44 : self.LeftMouseReleased,
            45 : self.RightMousePressed,
            46 : self.RightMouseReleased
        }

        self.shortcuts = {
            (Qt.Key_Z, Qt.ControlModifier) : self.undo,
            (Qt.Key_Delete, Qt.NoModifier) : self.deleteObject,
            (Qt.Key_S, Qt.AltModifier) : self.window.manipulator_scale,
            (Qt.Key_M, Qt.AltModifier) : self.window.manipulator_move,
            (Qt.Key_R, Qt.AltModifier) : self.window.manipulator_rotate,
            (Qt.Key_L, Qt.AltModifier) : self.linkObjects,
            (Qt.Key_L, Qt.ControlModifier|Qt.ShiftModifier) : self.unlinkObjects,
            #r.PyObjectEditDeselect: self.deselect,
            #r.PyObjectEditToggleMove: self.window.manipulator_move,#"ALT+M", #move
            #r.PyObjectEditToggleScale: self.window.manipulator_scale,#"ALT+S" #, #scale
            #r.Delete: self.deleteObject,
            #r.Undo: self.undo, 
            #r.PyDuplicateDrag: self.duplicateStart, 
            #r.ObjectLink: self.linkObjects,
            #r.ObjectUnlink: self.unlinkObjects,
        }
        
        self.resetManipulators()
        
        loader = QUiLoader()
        selectionfile = QFile(self.SELECTIONRECT)
        self.selection_rect = loader.load(selectionfile)
        rectprops = r.createUiWidgetProperty(2)
        #~ print type(rectprops), dir(rectprops)
        #print rectprops.WidgetType
        #uiprops.widget_name_ = "Selection Rect"
        
        #uiprops.my_size_ = QSize(width, height) #not needed anymore, uimodule reads it
        proxy = r.createUiProxyWidget(self.selection_rect, rectprops)
        uism = r.getUiSceneManager()
        uism.AddProxyWidget(proxy)
        proxy.setWindowFlags(0) #changing it to Qt::Widget
        
        self.selection_rect.setGeometry(0,0,0,0)
        self.selection_rect_startpos = None
        
        r.c = self #this is for using objectedit from command.py
        
        # Get world building modules python handler
        self.cpp_python_handler = r.getQWorldBuildingHandler()
        print self.cpp_python_handler
        if self.cpp_python_handler == None:
           r.logDebug("Could not aqquire world building service to object edit")
        
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
        #self.selection_box = None
        self.selection_rect_startpos = None
    
    def resetManipulators(self):
        self.manipulatorsInit = False
        self.manipulators = {}
        self.manipulators[self.MANIPULATE_MOVE] =  manipulator.MoveManipulator(self)
        self.manipulators[self.MANIPULATE_SCALE] =  manipulator.ScaleManipulator(self)
        self.manipulators[self.MANIPULATE_FREEMOVE] =  manipulator.FreeMoveManipulator(self)
        self.manipulators[self.MANIPULATE_ROTATE] =  manipulator.RotationManipulator(self)
        self.manipulator = self.manipulators[self.MANIPULATE_FREEMOVE]
 
    def baseselect(self, ent):
        ent, children = self.parentalCheck(ent)
        
        self.sel_activated = False
        self.worldstream.SendObjectSelectPacket(ent.id)
        #self.updateSelectionBox(ent)
        self.highlight(ent)
        self.changeManipulator(self.MANIPULATE_FREEMOVE)

        #print "selected", ent
        
        return ent, children
        
    def parentalCheck(self, ent):
        children = []
        
        while 1:
            try:
                qprim = ent.prim
            except AttributeError:
                # we come here when parent has no EC_opensimprim component
                break

            if qprim.ParentId != 0:
                #~ r.logInfo("Entity had a parent, lets pick that instead!")
                # get the parent entity, and if it is editable set it to ent.
                # on next loop we get prim from it and from that we get children.
                temp_ent = r.getEntity(qprim.ParentId)
                if not temp_ent.editable:
                    # not a prim, so not selecting all children
                    break
                else:
                    ent = temp_ent
            else:
                #~ r.logInfo("Entity had no parent, maybe it has children?")
                # either we get children or not :) But this is the 'parent' in either case
                children = qprim.GetChildren()
                break
        return ent, children
        
    def select(self, ent):
        self.deselect_all()
        ent, children = self.baseselect(ent)
        self.sels.append(ent)
        self.window.selected(ent, False) 
        self.canmove = True
        
        if self.cpp_python_handler != None:
            self.cpp_python_handler.ObjectSelected(ent)
        
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
            self.highlight(child)
            #self.sels.append(child)
            
    def deselect(self, ent):
        self.remove_highlight(ent)
        for _ent in self.sels: #need to find the matching id in list 'cause PyEntity instances are not reused yet XXX
            if _ent.id == ent.id:
                self.sels.remove(_ent)
                self.window.deselectSelection(_ent.id)
            
    def deselect_all(self):
        if len(self.sels) > 0:
            #XXX might need something here?!
            
            for ent in self.sels:
                self.remove_highlight(ent)
            self.sels = []
            #self.hideSelector()
            
            self.hideManipulator() #manipulator

            self.prev_mouse_abs_x = 0
            self.prev_mouse_abs_y = 0
            self.canmove = False

            self.window.deselected()
            
    # def updateSelectionBox(self, ent): 
#         if ent is not None:
#             bb = list(ent.boundingbox)
#             height = abs(bb[4] - bb[1]) 
#             width = abs(bb[3] - bb[0])
#             depth = abs(bb[5] - bb[2])

#             self.selection_box.placeable.Position = ent.placeable.Position
            
#             self.selection_box.placeable.Scale = Vec(height, width, depth)#depth, width, height
#             self.selection_box.placeable.Orientation = ent.placeable.Orientation
                
    def highlight(self, ent):
        try:
            ent.highlight
        except AttributeError:
            ent.createComponent("EC_Highlight")
            #print "created a new Highlight component"

        h = ent.highlight
        #print type(h), h
    
        if not h.IsVisible():
            h.Show()
            
        else:
            r.logInfo("objectedit.highlight called for an already hilited entity: %d" % ent.id)
            
    def remove_highlight(self, ent):
        try:
            h = ent.highlight
        except AttributeError:
            r.logInfo("objectedit.remove_highlight called for a non-hilited entity: %d" % ent.id)
        else:
            h.Hide()        

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

#     def hideSelector(self):
#         try: #XXX! without this try-except, if something is selected, the viewer will crash on exit
#             if self.selection_box is not None:
#                 self.selection_box.placeable.Scale = Vec(0.0, 0.0, 0.0)
#                 self.selection_box.placeable.Position = Vec(0.0, 0.0, 0.0)
#         except RuntimeError, e:
#             r.logDebug("hideSelector failed")
        
    def getSelectedObjectIds(self):
        ids = []
        for ent in self.sels:
            qprim = ent.prim
            children = qprim.GetChildren()
            for child_id in children:
                #child =  r.getEntity(int(child_id))
                id = int(child_id)
                if id not in ids:
                    ids.append(id)
            ids.append(ent.id)
        return ids
    
    def linkObjects(self):
        ids = self.getSelectedObjectIds()
        self.worldstream.SendObjectLinkPacket(ids)
        self.deselect_all()
        
    def unlinkObjects(self):
        ids = self.getSelectedObjectIds()
        self.worldstream.SendObjectDelinkPacket(ids)
        self.deselect_all()
        
    def LeftMousePressed(self, mouseinfo):
        #r.logDebug("LeftMousePressed") #, mouseinfo, mouseinfo.x, mouseinfo.y
        #r.logDebug("point " + str(mouseinfo.x) + "," + str(mouseinfo.y))
        
        self.dragStarted(mouseinfo) #need to call this to enable working dragging
        
#         if self.selection_box is None:
#             self.selection_box = r.createEntity("Selection.mesh", -10000)
        
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
        self.usingManipulator = True

        if ent is not None:
            #print "Got entity:", ent, ent.editable
            if not self.manipulator.compareIds(ent.id) and ent.editable: #ent.id != self.selection_box.id and 
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
            self.deselect_all()
            
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
        self.manipulator.showManipulator(self.sels)
        self.usingManipulator = False
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
            if ent is not None:                
                #print "Got entity:", ent.id
                for entity in self.sels:
                    if entity.id == ent.id:
                        found = True #clicked on an already selected entity
                        #print "multiselect clicked entity is already in selection"
                
                #if self.active is None or self.active.id != ent.id: #a diff ent than prev sel was changed  
                if self.validId(ent.id):
                    if not found:
                        #print "new ent to add to multiselect found:", ent.id
                        self.multiselect(ent)
                    else: #remove this entity which was previously in the selection
                        self.deselect(ent)
                    self.canmove = True
                        
            #r.logInfo(str(self.sels))
    def validId(self, id):
        if id != 0 and id > 50: #terrain seems to be 3 and scene objects always big numbers, so > 50 should be good, though randomly created local entities can get over 50...
            if id != r.getUserAvatarId(): #add other avatar id's check
                if not self.manipulator.compareIds(id):  #and id != self.selection_box.id:
                    return True
        return False
        
    def RightMouseReleased(self, mouseinfo):
        #r.logInfo("rightmouse up")
        self.right_button_down = False
        
    def on_mouseclick(self, click_id, mouseinfo):
        print click_id, mouseinfo
        if self.windowActive: #XXXnot self.canvas.IsHidden():
            if self.mouse_events.has_key(click_id):
                self.mouse_events[click_id](mouseinfo)
                #~ r.logInfo("on_mouseclick %d %s" % (click_id, self.mouse_events[click_id]))
        #r.logInfo("on_mouseclick %d" % (click_id))

    def on_mousemove(self, event_id, mouseinfo):
        """for hilighting manipulator parts when hovering over them"""
        #print "m"
        if self.windowActive:# and event_id == :
            #~ print "m"
            results = []
            results = r.rayCast(mouseinfo.x, mouseinfo.y)
            if results is not None and results[0] != 0:
                id = results[0]
                
                if self.manipulator.compareIds(id):
                    self.manipulator.highlight(results)
            else:
                self.manipulator.resethighlight()
        
                
    def on_mousedrag(self, move_id, mouseinfo):
        """dragging objects around - now free movement based on view,
        dragging different axis etc in the manipulator to be added."""
        #print "mousedrag:", move_id, mouseinfo
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
                    self.selection_rect.setGeometry(rectx, recty, rectwidth, rectheight)
                    self.selection_rect.show() #XXX change?
                    
                    #r.logInfo("The selection rect was at: (" +str(rectx) + ", " +str(recty) + ") and size was: (" +str(rectwidth) +", "+str(rectheight)+")")
                    rect = self.selection_rect.rect #0,0 - x, y
                    rect.translate(mouseinfo.x, mouseinfo.y)
                    #print rect.left(), rect.top(), rect.right(), rect.bottom()
                    rend = r.getQRenderer()
                    hits = rend.FrustumQuery(rect) #the wish
                    #hits = r.frustumQuery(rect.left(), rect.top(), rect.right(), rect.bottom()) #current workaround
                    print hits

                else:
                    if self.duplicateDragStart:
                        for ent in self.sels:
                            self.worldstream.SendObjectDuplicatePacket(ent.id, ent.prim.UpdateFlags, 0, 0, 0) #nasty hardcoded offset
                        self.duplicateDragStart = False
                            
                    ent = self.active
                    #print "on_mousemove + hold:", mouseinfo
                    if ent is not None and self.sel_activated and self.canmove:
                        self.dragging = True

                        self.manipulator.manipulate(self.sels, movedx, movedy)
                        
                        self.prev_mouse_abs_x = mouse_abs_x
                        self.prev_mouse_abs_y = mouse_abs_y
                        
                        self.window.update_guivals(ent)
   
    def on_input(self, evid):
        #print "input", evid
        if self.windowActive:
            if evid in self.shortcuts:#self.shortcuts.has_key((keycode, keymod)):
                self.keypressed = True
                self.shortcuts[evid]()
                return True
        
    def on_inboundnetwork(self, evid, name):
        return False
        #print "editgui got an inbound network event:", id, name

    def undo(self):
        #print "undo clicked"
        ent = self.active
        if ent is not None:
            self.worldstream.SendObjectUndoPacket(ent.prim.FullId)
            self.window.update_guivals(ent)
            self.modified = False
            self.deselect_all()

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
            self.worldstream.SendObjectDuplicatePacket(ent.id, ent.prim.UpdateFlags, 1, 1, 1) #nasty hardcoded offset
        
    def duplicateStart(self):
        self.duplicateDragStart = True
        
    def createObject(self):
        avatar_id = r.getUserAvatarId()
        avatar = r.getEntity(avatar_id)
        pos = avatar.placeable.Position#r.getUserAvatarPos()

        # TODO determine what is right in front of avatar and use that instead
        start_x = pos.x() + .5
        start_y = pos.y() + .5
        start_z = pos.z()

        self.worldstream.SendObjectAddPacket(start_x, start_y, start_z)

    def deleteObject(self):
        if self.active is not None:
            for ent in self.sels:
                #r.logInfo("deleting " + str(ent.id))
                ent, children = self.parentalCheck(ent)
                for child_id in children:
                    child = r.getEntity(int(child_id))
                    #~ self.window.addToList(child)
                    #~ print "deleting", child
                    #~ self.worldstream.SendObjectDeRezPacket(child.id, r.getTrashFolderId())
                    self.window.objectDeleted(str(child.id))
                #~ if len(children) == 0:
                self.worldstream.SendObjectDeRezPacket(ent.id, r.getTrashFolderId())
                self.window.objectDeleted(str(ent.id))
                #~ else:
                    #~ r.logInfo("trying to delete a parent, need to fix this!")
            
            self.manipulator.hideManipulator()
            #self.hideSelector()        
            self.deselect_all()
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
            qpos = ent.placeable.Position
            pos = list((qpos.x(), qpos.y(), qpos.z())) #should probably wrap Vector3, see test_move.py for refactoring notes. 
    
            if not self.float_equal(pos[i],v):
                pos[i] = v
                #converted to list to have it mutable
                newpos = Vec(pos[0], pos[1], pos[2])
                ent.placeable.Position = newpos
                ent.network.Position = newpos
                self.manipulator.moveTo(self.sels)

                self.modified = True
                if not self.dragging:
                    r.networkUpdate(ent.id)
            
    def changescale(self, i, v):
        ent = self.active
        if ent is not None:
            qscale = ent.placeable.Scale
            oldscale = list((qscale.x(), qscale.y(), qscale.z()))
            scale = list((qscale.x(), qscale.y(), qscale.z()))
                
            if not self.float_equal(scale[i],v):
                scale[i] = v
                if self.window.mainTab.scale_lock.checked:
                    #XXX BUG does wrong thing - the idea was to maintain aspect ratio
                    diff = scale[i] - oldscale[i]
                    for index in range(len(scale)):
                        #print index, scale[index], index == i
                        if index != i:
                            scale[index] += diff
                
                ent.placeable.Scale = Vec(scale[0], scale[1], scale[2])
                
                if not self.dragging:
                    r.networkUpdate(ent.id)
                
                #self.window.update_scalevals(scale)
                
                self.modified = True

                #self.updateSelectionBox(ent)
            
    def changerot(self, i, v):
        #XXX NOTE / API TODO: exceptions in qt slots (like this) are now eaten silently
        #.. apparently they get shown upon viewer exit. must add some qt exc thing somewhere
        #print "pos index %i changed to: %f" % (i, v[i])
        ent = self.active
        if ent is not None and not self.usingManipulator:
            quat = conv.euler_to_quat(v)
            # convert between conversions.Quat tuple (x,y,z,w) format and QQuaternion (w,x,y,z)
            # TODO: it seems that visualisation compared to what we give/understand on ob edit
            # level is shifted. For now leave this 'shift' in, but need to investigate later. At
            # least visual changes triggered through ob edit window widgets seem to correspond
            # better to what one expects.
            ort = Quat(quat[3], quat[1], quat[2], quat[0])
            ent.placeable.Orientation = ort
            ent.network.Orientation = ort
            if not self.dragging:
                r.networkUpdate(ent.id)
                
            self.modified = True

    def getActive(self):
        if len(self.sels) > 0:
            ent = self.sels[-1]
            return ent
        return None
        
    active = property(getActive)
    
    def on_exit(self):
        r.logInfo("Object Edit exiting...")

        self.deselect_all()
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
                self.deselect_all()
        else:
            self.deselect_all()
            
    def update(self, time):
        #print "here", time
        if self.windowActive:
            self.time += time
            if self.sels:
                ent = self.active
                if self.time > self.UPDATE_INTERVAL:
                    try:
                        #sel_pos = self.selection_box.placeable.Position
                        arr_pos = self.manipulator.getManipulatorPosition()
                        ent_pos = ent.placeable.Position
                        #if sel_pos != ent_pos:
                        self.time = 0 #XXX NOTE: is this logic correct?
                        #    self.selection_box.placeable.Position = ent_pos
                        if arr_pos != ent_pos:
                            self.manipulator.moveTo(self.sels)
                    except RuntimeError, e:
                        r.logDebug("update: scene not found")
   
    def on_logout(self, id):
        r.logInfo("Object Edit resetting due to Logout.")
        self.deselect_all()
        self.sels = []
        self.selection_box = None
        self.resetValues()
        self.resetManipulators()
        
    def setUseLocalTransform(self, local):
        self.useLocalTransform = local

import rexviewer as r

import PythonQt
from PythonQt.QtGui import QTreeWidgetItem, QSizePolicy, QIcon, QHBoxLayout, QComboBox
from PythonQt.QtUiTools import QUiLoader
from PythonQt.QtCore import QFile, QSize
from conversions import quat_to_euler, euler_to_quat #for euler - quat -euler conversions
from vector3 import Vector3 #for view based editing calcs now that Vector3 not exposed from internals

try:
    lines
    buttons 
except: #first run
    try:
        import lines
        import buttons
    except ImportError, e:
        print "couldn't load lines and buttons:", e
else:
    lines = reload(lines)
    buttons = reload(buttons)

PRIMTYPES = {
    "45": "Material",
    "0": "Texture"
}

class ObjectEditWindow:
    UIFILE = "pymodules/objectedit/editobject.ui"
    
    ICON_OK = "pymodules/objectedit/ok.png"
    ICON_CANCEL = "pymodules/objectedit/cancel.png" 
    
    def __init__(self, creator):
        self.creator = creator
        loader = QUiLoader()
        uifile = QFile(self.UIFILE)

        ui = loader.load(uifile)
        width = ui.size.width()
        height = ui.size.height()
        
        #if not DEV:
        uism = r.getUiSceneManager()
        uiprops = r.createUiWidgetProperty()
        uiprops.widget_name_ = "Object Edit"
        #uiprops.my_size_ = QSize(width, height) #not needed anymore, uimodule reads it
        self.proxywidget = r.createUiProxyWidget(ui, uiprops)

        if not uism.AddProxyWidget(self.proxywidget):
            print "Adding the ProxyWidget to the bar failed."
        
        self.widget = ui
        self.tabwidget = ui.findChild("QTabWidget", "MainTabWidget")

        self.mainTab = ui.findChild("QWidget", "MainFrame")
        self.materialTab = ui.findChild("QWidget", "MaterialsTab")
        self.tabwidget.setTabEnabled(1, False)
        self.materialTabFormWidget = self.materialTab.formLayoutWidget
        self.mainTab.label.text = "<none>"

        self.meshline = lines.MeshAssetidEditline(creator) 
        self.meshline.name = "meshLineEdit"

        button_ok = self.getButton("Apply", self.ICON_OK, self.meshline, self.meshline.applyAction)
        button_cancel = self.getButton("Cancel", self.ICON_CANCEL, self.meshline, self.meshline.cancelAction)
        
        box = self.mainTab.findChild("QHBoxLayout", "meshLine")
        box.addWidget(self.meshline)
        box.addWidget(button_ok)
        box.addWidget(button_cancel)
        
        self.propedit = r.getPropertyEditor()
        self.tabwidget.addTab(self.propedit, "Properties")
        self.tabwidget.setTabEnabled(2, False)
        
        def poschanger(i):
            def pos_at_index(v):
                self.changepos(i, v)
            return pos_at_index
        for i, poswidget in enumerate([self.mainTab.xpos, self.mainTab.ypos, self.mainTab.zpos]):
            poswidget.connect('valueChanged(double)', poschanger(i))

        def rotchanger(i):
            def rot_at_index(v):
                self.changerot(i, v)
            return rot_at_index
        for i, rotwidget in enumerate([self.mainTab.rot_x, self.mainTab.rot_y, self.mainTab.rot_z]):
            rotwidget.connect('valueChanged(double)', rotchanger(i))
        
        def scalechanger(i):
            def scale_at_index(v):
                self.changescale(i, v)
            return scale_at_index
        for i, scalewidget in enumerate([self.mainTab.scalex, self.mainTab.scaley, self.mainTab.scalez]):
            scalewidget.connect('valueChanged(double)', scalechanger(i))
        
        self.mainTab.treeWidget.connect('clicked(QModelIndex)', self.itemActivated)
        
        self.proxywidget.connect('Visible(bool)', self.creator.on_hide)
        self.tabwidget.connect('currentChanged(int)', self.tabChanged)

        self.meshline.connect('textEdited(QString)', button_ok.lineValueChanged)
        self.meshline.connect('textEdited(QString)', button_cancel.lineValueChanged)
        
        self.mainTab.findChild("QPushButton", "newObject").connect('clicked()', self.createObject)
        self.mainTab.findChild("QPushButton", "deleteObject").connect('clicked()', self.deleteObject)
        self.mainTab.findChild("QPushButton", "duplicate").connect('clicked()', self.duplicate)
        
        self.mainTab.findChild("QPushButton", "undo").connect('clicked()', self.undo)
        
        self.mainTab.findChild("QToolButton", "move_button").connect('clicked()', self.manipulator_move)
        self.mainTab.findChild("QToolButton", "scale_button").connect('clicked()', self.manipulator_scale)
        self.mainTab.findChild("QToolButton", "rotate_button").connect('clicked()', self.manipulator_rotate)

        self.mainTabList = {}
        
    def undo(self):
        #print "undo clicked"
        ent = self.creator.sel
        if ent is not None:
            self.worldstream.SendObjectUndoPacket(ent.uuid)
            self.update_guivals(ent)
            self.modified = False

    #~ def redo(self):
        #~ #print "redo clicked"
        #~ ent = self.sel
        #~ if ent is not None:
            #~ #print ent.uuid
            #~ #worldstream = r.getServerConnection()
            #~ self.worldstream.SendObjectRedoPacket(ent.uuid)
            #~ #self.sel = None
            #~ self.update_guivals()
            #~ self.modified = False
            
    def duplicate(self):
        #print "duplicate clicked"
        ent = self.creator.sel
        if ent is not None:
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
        ent = self.creator.sel
        if ent is not None:
            self.worldstream.SendObjectDeRezPacket(ent.id, r.getTrashFolderId())
            self.creator.manipulator.hideManipulator()
            self.hideSelector()
            id, tWid = self.mainTabList.pop(str(ent.id))
            tWid.delete()
            self.deselect()
            self.creator.sel = None

    def update_guivals(self, ent):
        #ent = self.creator.sel
        x, y, z = ent.pos
        self.mainTab.xpos.setValue(x)
        self.mainTab.ypos.setValue(y)
        self.mainTab.zpos.setValue(z)
            
        x, y, z = ent.scale
        self.mainTab.scalex.setValue(x)
        self.mainTab.scaley.setValue(y)
        self.mainTab.scalez.setValue(z)
            
        #from quat to euler x.y,z
        euler = quat_to_euler(ent.orientation)
        self.mainTab.rot_x.setValue(euler[0])
        self.mainTab.rot_y.setValue(euler[1])
        self.mainTab.rot_z.setValue(euler[2])        
         
        self.creator.selection_box.pos = ent.pos
        self.creator.selection_box.orientation = ent.orientation
    
    def reset_guivals(self):
        self.mainTab.xpos.setValue(0)
        self.mainTab.ypos.setValue(0)
        self.mainTab.zpos.setValue(0)

        self.mainTab.scalex.setValue(0)
        self.mainTab.scaley.setValue(0)
        self.mainTab.scalez.setValue(0)

        self.mainTab.rot_x.setValue(0)
        self.mainTab.rot_y.setValue(0)
        self.mainTab.rot_z.setValue(0)  
    
    def deselected(self):
        self.mainTab.label.text = "<none>"  
        self.tabwidget.setTabEnabled(1, False)
        self.tabwidget.setTabEnabled(2, False)
        
        self.meshline.update_text("")
        self.reset_guivals()
        
        self.creator.window.mainTab.move_button.setChecked(False)
        self.creator.window.mainTab.rotate_button.setChecked(False)
        self.creator.window.mainTab.scale_button.setChecked(False)
        
    def updateMaterialTab(self, ent):
        if ent is not None:
            self.clearDialogForm()
            qprim = r.getQPrim(ent.id)
            mats = qprim.Materials
            #print mats#, r.formwidget.formLayout.children() 

            #for tuple in sorted(mats.itervalues()):
            for i in range(len(mats)):
                index = str(i)
                tuple = mats[index]
                line = lines.UUIDEditLine(self.creator)#QLineEdit()
                line.update_text(tuple[1])
                line.name = index
                asset_type = tuple[0]
                    
                combobox = QComboBox()
                for text in PRIMTYPES.itervalues():
                    combobox.addItem(text)
                
                if PRIMTYPES.has_key(asset_type):
                    realIndex = combobox.findText(PRIMTYPES[asset_type])
                    #print realIndex, asset_type, PRIMTYPES[asset_type]
                    combobox.setCurrentIndex(realIndex)
                
                applyButton = self.getButton("materialApplyButton", self.ICON_OK, line, line.applyAction)
                cancelButton = self.getButton("materialCancelButton", self.ICON_CANCEL, line, line.cancelAction)
                line.index = index
                line.combobox = combobox
                line.connect('textEdited(QString)', applyButton.lineValueChanged)
                line.connect('textEdited(QString)', cancelButton.lineValueChanged)
                
                box = QHBoxLayout()
                box.addWidget(line)
                box.addWidget(applyButton)
                box.addWidget(cancelButton)
                
                self.materialTabFormWidget.materialFormLayout.addRow(combobox, box)
                

    def clearDialogForm(self):
        children = self.materialTabFormWidget.children()
        for child in children:
            if child.name != "materialFormLayout": #dont want to remove the actual form layout from the widget
                self.materialTabFormWidget.materialFormLayout.removeWidget(child)
                child.delete()
        
        children = self.materialTabFormWidget.findChildren("QHBoxLayout")
        for child in children:
            self.materialTabFormWidget.materialFormLayout.removeItem(child)
            child.delete()

    def itemActivated(self, item=None): #the item from signal is not used, same impl used by click
        #print "Got the following item index...", item, dir(item), item.data, dir(item.data) #we has index, now what? WIP
        current = self.mainTab.treeWidget.currentItem()
        text = current.text(0)
        if self.mainTabList.has_key(text):
            self.select(self.mainTabList[text][0])
    
    def getButton(self, name, iconname, line, action):
        size = QSize(16, 16)
        button = buttons.PyPushButton()
        icon = QIcon(iconname)
        icon.actualSize(size)
        button.setSizePolicy(QSizePolicy.Fixed, QSizePolicy.Fixed)
        button.setMaximumSize(size)
        button.setMinimumSize(size)
        button.text = ""
        button.name = name
        button.setIcon(icon)
        button.setFlat(True)
        button.setEnabled(False)
        button.connect('clicked()', action)
        line.buttons.append(button)
        return button
        
    def tabChanged(self, index):
        if index == 1:
            self.updateMaterialTab(self.creator.sel)
        #~ elif index == 0:
            #~ print "Object Edit"
        #~ else:
            #~ print "nothing found!"
            
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
        ent = self.creator.sel
        
        if ent is not None:
            #print "sel pos:", ent.pos, pos[i], v
            pos = list(ent.pos) #should probably wrap Vector3, see test_move.py for refactoring notes. 
    
            if not self.float_equal(pos[i],v):
                pos[i] = v
                #converted to list to have it mutable
                ent.pos = pos[0], pos[1], pos[2] #XXX API should accept a list/tuple too .. or perhaps a vector type will help here too
                #print "=>", ent.pos
                self.creator.manipulator.moveTo(pos)
                #self.creator.selection_box.pos = pos[0], pos[1], pos[2]

                self.mainTab.xpos.setValue(pos[0])
                self.mainTab.ypos.setValue(pos[1])
                self.mainTab.zpos.setValue(pos[2])
                self.modified = True
                if not self.creator.dragging:
                    r.networkUpdate(ent.id)
            
    def changescale(self, i, v):
        ent = self.creator.sel
        if ent is not None:
            oldscale = list(ent.scale)
            scale = list(ent.scale)
                
            if not self.float_equal(scale[i],v):
                scale[i] = v
                if self.mainTab.scale_lock.checked:
                    #XXX BUG does wrong thing - the idea was to maintain aspect ratio
                    diff = scale[i] - oldscale[i]
                    for index in range(len(scale)):
                        #print index, scale[index], index == i
                        if index != i:
                            scale[index] += diff
                
                ent.scale = scale[0], scale[1], scale[2]
                
                if not self.creator.dragging:
                    r.networkUpdate(ent.id)
                
                self.mainTab.scalex.setValue(scale[0])
                self.mainTab.scaley.setValue(scale[1])
                self.mainTab.scalez.setValue(scale[2])
                self.modified = True

                self.update_selection()
            
    def changerot(self, i, v):
        #XXX NOTE / API TODO: exceptions in qt slots (like this) are now eaten silently
        #.. apparently they get shown upon viewer exit. must add some qt exc thing somewhere
        #print "pos index %i changed to: %f" % (i, v)
        ent = self.creator.sel
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
                if not self.creator.dragging:
                    r.networkUpdate(ent.id)
                    
                self.modified = True

                self.creator.selection_box.orientation = ort

    def manipulator_move(self):
        #~ if self.creator.keypressed:
            #~ self.creator.keypressed = False
            #~ if not self.mainTab.move_button.isChecked():
                #~ self.mainTab.move_button.setChecked(True)
            #~ else:
                #~ self.mainTab.move_button.setChecked(False)
        
        freemove = False

        if not self.mainTab.move_button.isChecked():
            freemove = True
            self.creator.manipulator.hideManipulator()
        else: #activated
            if self.creator.sel is not None:
                self.mainTab.scale_button.setChecked(False)
                self.mainTab.rotate_button.setChecked(False)
                self.creator.changeManipulator(self.creator.MANIPULATE_MOVE)   
            else:
                self.mainTab.move_button.setChecked(False)
                freemove = True
        
        #~ if freemove:
            #~ self.creator.changeManipulator(self.creator.MANIPULATE_FREEMOVE)
        
    def manipulator_scale(self):
        #~ if self.creator.keypressed:
            #~ self.creator.keypressed = False
            #~ if not self.mainTab.scale_button.isChecked():
                #~ self.mainTab.scale_button.setChecked(True)
            #~ else:
                #~ self.mainTab.scale_button.setChecked(False)
                
        freemove = False
        if not self.mainTab.scale_button.isChecked():
            freemove = True
            self.creator.manipulator.hideManipulator()
        else: #activated
            if self.creator.sel is not None:
                self.mainTab.move_button.setChecked(False)
                self.mainTab.rotate_button.setChecked(False)
                self.creator.changeManipulator(self.creator.MANIPULATE_SCALE)
            else:
                self.mainTab.scale_button.setChecked(False)
                freemove = True
        
        #~ if freemove:
            #~ self.creator.changeManipulator(self.creator.MANIPULATE_FREEMOVE)
            
    def manipulator_rotate(self):
        if self.creator.keypressed:
            self.creator.keypressed = False
            if not self.mainTab.rotate_button.isChecked():
                self.mainTab.rotate_button.setChecked(True)
            else:
                self.mainTab.rotate_button.setChecked(False)
                
        freemove = False
        if not self.mainTab.rotate_button.isChecked():
            freemove = True
            self.creator.manipulator.hideManipulator()
            r.logInfo("not activated, something")
        else: #activated
            if self.creator.sel is not None:
                r.logInfo("activated, has selectiong")
                self.creator.changeManipulator(self.creator.MANIPULATE_ROTATE)
                self.mainTab.scale_button.setChecked(False)
                self.mainTab.move_button.setChecked(False)
            else:
                r.logInfo("activated, no selectiong")
                self.mainTab.rotate_button.setChecked(False) 
                freemove = True
        
        #~ if freemove:
            #~ self.creator.changeManipulator(self.creator.MANIPULATE_FREEMOVE)
            
    def selected(self, ent):
        if not self.mainTabList.has_key(str(ent.id)):
            tWid = QTreeWidgetItem(self.mainTab.treeWidget)
            id = ent.id
            tWid.setText(0, id)
            
            self.mainTabList[str(id)] = (ent, tWid)
        
            """show the id and name of the object. name is sometimes empty it seems. 
                swoot: actually, seems like the name just isn't gotten fast enough or 
                something.. next time you click on the same entity, it has a name."""
            name = ent.name
            if name == "":
                name = "n/a"
            self.mainTab.label.text = "%d (name: %s)" % (ent.id, name)
            
            self.meshline.update_text(ent.mesh)
            
            qprim = r.getQPrim(ent.id)
            self.propedit.setObject(qprim)
            self.tabwidget.setTabEnabled(2, True)
            
        self.updateMaterialTab(ent)
        self.tabwidget.setTabEnabled(1, True)
        self.update_guivals(ent)
        
    def on_exit(self):
        self.proxywidget.hide()
        uism = r.getUiSceneManager()
        uism.RemoveProxyWidgetFromScene(self.proxywidget)
        uism.RemoveProxyWidgetFromScene(self.propeditwidget)
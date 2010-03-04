import rexviewer as r

import PythonQt
from PythonQt.QtGui import QTreeWidgetItem, QSizePolicy, QIcon, QHBoxLayout, QComboBox
from PythonQt.QtUiTools import QUiLoader
from PythonQt.QtCore import QFile, QSize
from conversions import quat_to_euler #for euler - quat -euler conversions
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
    
    def __init__(self, controller):
        self.controller = controller
        loader = QUiLoader()
        uifile = QFile(self.UIFILE)

        ui = loader.load(uifile)
        width = ui.size.width()
        height = ui.size.height()
        
        #if not DEV:
        uism = r.getUiSceneManager()
        uiprops = r.createUiWidgetProperty(1) #1 is ModuleWidget, shown at toolbar
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

        self.meshline = lines.MeshAssetidEditline(controller) 
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
                self.controller.changepos(i, v)
            return pos_at_index
        for i, poswidget in enumerate([self.mainTab.xpos, self.mainTab.ypos, self.mainTab.zpos]):
            poswidget.connect('valueChanged(double)', poschanger(i))

        def rotchanger(i):
            def rot_at_index(v):
                self.controller.changerot(i, v)
            return rot_at_index
        for i, rotwidget in enumerate([self.mainTab.rot_x, self.mainTab.rot_y, self.mainTab.rot_z]):
            rotwidget.connect('valueChanged(double)', rotchanger(i))
        
        def scalechanger(i):
            def scale_at_index(v):
                self.controller.changescale(i, v)
            return scale_at_index
        for i, scalewidget in enumerate([self.mainTab.scalex, self.mainTab.scaley, self.mainTab.scalez]):
            scalewidget.connect('valueChanged(double)', scalechanger(i))
        
        self.mainTab.treeWidget.connect('clicked(QModelIndex)', self.itemActivated)
        self.mainTab.treeWidget.connect('activated(QModelIndex)', self.itemActivated)
        
        self.proxywidget.connect('Visible(bool)', self.controller.on_hide)
        #self.tabwidget.connect('currentChanged(int)', self.tabChanged)

        self.meshline.connect('textEdited(QString)', button_ok.lineValueChanged)
        self.meshline.connect('textEdited(QString)', button_cancel.lineValueChanged)
        
        self.mainTab.findChild("QPushButton", "newObject").connect('clicked()', self.controller.createObject)
        self.mainTab.findChild("QPushButton", "deleteObject").connect('clicked()', self.controller.deleteObject)
        self.mainTab.findChild("QPushButton", "duplicate").connect('clicked()', self.controller.duplicate)
        
        self.mainTab.findChild("QPushButton", "undo").connect('clicked()', self.controller.undo)
        
        self.mainTab.findChild("QToolButton", "move_button").connect('clicked()', self.manipulator_move)
        self.mainTab.findChild("QToolButton", "scale_button").connect('clicked()', self.manipulator_scale)
        self.mainTab.findChild("QToolButton", "rotate_button").connect('clicked()', self.manipulator_rotate)

        self.mainTabList = {}
        
        self.currentlySelectedTreeWidgetItem = []

    def update_guivals(self, ent):   
        #from quat to euler x.y,z
        if ent is not None:
            self.update_posvals(ent.pos)
            self.update_scalevals(ent.scale)
            self.update_rotvals(ent.orientation)
            self.controller.updateSelectionBox(ent) #PositionAndOrientation(ent)
        
    def update_scalevals(self, scale):
        x, y, z = scale
        self.mainTab.scalex.setValue(x)
        self.mainTab.scaley.setValue(y)
        self.mainTab.scalez.setValue(z)
        
    def update_posvals(self, pos):
        x, y, z = pos
        self.mainTab.xpos.setValue(x)
        self.mainTab.ypos.setValue(y)
        self.mainTab.zpos.setValue(z)
        
    def update_rotvals(self, rot):
        euler = quat_to_euler(rot)
        self.mainTab.rot_x.setValue(euler[0])
        self.mainTab.rot_y.setValue(euler[1])
        self.mainTab.rot_z.setValue(euler[2])   
    
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
        
        self.untoggleButtons()
        
        self.unsetSelection()
        
    def unsetSelection(self):
        for tuples in self.mainTabList.values():
            tWid = tuples[1]
            tWid.setSelected(False)
        
        self.currentlySelectedTreeWidgetItem = []
        
    def deselectSelection(self, id):
        for listid in self.mainTabList.keys():
            if listid == str(id):
                tuple = self.mainTabList[listid]
                tWid = tuple[1]
                tWid.setSelected(False)
            
    def updateMaterialTab(self, ent):
        #ent = self.controller.active
        if ent is not None:
            self.clearDialogForm()
            qprim = r.getQPrim(ent.id)
            mats = qprim.Materials
            #print mats#, r.formwidget.formLayout.children() 

            #for tuple in sorted(mats.itervalues()):
            for i in range(len(mats)):
                index = str(i)
                tuple = mats[index]
                line = lines.UUIDEditLine(self.controller)#QLineEdit()
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
                
            self.tabwidget.setTabEnabled(1, True)

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
            ent = self.mainTabList[text][0]
            self.controller.select(ent)
    
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
        
    #~ def tabChanged(self, index):
        #~ if index == 1:
            #~ self.updateMaterialTab()
        ##~ elif index == 0:
            ##~ print "Object Edit"
        ##~ else:
            ##~ print "nothing found!"
            
    def manipulator_move(self):
        ent = self.controller.active
        if self.controller.keypressed:
            self.controller.keypressed = False
            if not self.mainTab.move_button.isChecked():
                self.mainTab.move_button.setChecked(True)
            else:
                self.mainTab.move_button.setChecked(False)
        
        freemove = False

        if not self.mainTab.move_button.isChecked():
            freemove = True
            self.controller.hideManipulator()
        else: #activated
            if ent is not None:
                self.mainTab.scale_button.setChecked(False)
                self.mainTab.rotate_button.setChecked(False)
                self.controller.changeManipulator(self.controller.MANIPULATE_MOVE)   
            else:
                self.mainTab.move_button.setChecked(False)
                freemove = True
        
        if freemove:
            self.controller.changeManipulator(self.controller.MANIPULATE_FREEMOVE)
        
    def manipulator_scale(self):
        ent = self.controller.active
        if self.controller.keypressed:
            self.controller.keypressed = False
            if not self.mainTab.scale_button.isChecked():
                self.mainTab.scale_button.setChecked(True)
            else:
                self.mainTab.scale_button.setChecked(False)
                
        freemove = False
        if not self.mainTab.scale_button.isChecked():
            freemove = True
            self.controller.hideManipulator()
        else: #activated
            if ent is not None:
                self.mainTab.move_button.setChecked(False)
                self.mainTab.rotate_button.setChecked(False)
                self.controller.changeManipulator(self.controller.MANIPULATE_SCALE)
            else:
                self.mainTab.scale_button.setChecked(False)
                freemove = True
        
        if freemove:
            self.controller.changeManipulator(self.controller.MANIPULATE_FREEMOVE)
            
    def manipulator_rotate(self):
        ent = self.controller.active
        
        if self.controller.keypressed:
            self.controller.keypressed = False
            if not self.mainTab.rotate_button.isChecked():
                self.mainTab.rotate_button.setChecked(True)
            else:
                self.mainTab.rotate_button.setChecked(False)
                
        freemove = False
        if not self.mainTab.rotate_button.isChecked():
            freemove = True
            self.controller.hideManipulator()
            r.logInfo("not activated, something")
        else: #activated
            if ent is not None:
                r.logInfo("activated, has selectiong")
                self.controller.changeManipulator(self.controller.MANIPULATE_ROTATE)
                self.mainTab.scale_button.setChecked(False)
                self.mainTab.move_button.setChecked(False)
            else:
                r.logInfo("activated, no selectiong")
                self.mainTab.rotate_button.setChecked(False) 
                freemove = True
        
        if freemove:
            self.controller.changeManipulator(self.controller.MANIPULATE_FREEMOVE)
            
    def selected(self, ent, keepold=False):
        self.untoggleButtons()
        
        if not keepold:
            self.unsetSelection()
        
        self.addToList(ent)
        
        self.highlightEntityFromList(ent)
            
        self.showName(ent)
        
        self.meshline.update_text(ent.mesh)
        
        self.updateMaterialTab(ent)

        self.updatePropertyEditor(ent)

        self.update_guivals(ent)
    
    def updatePropertyEditor(self, ent):
        qprim = r.getQPrim(ent.id)
        if qprim is not None:
            self.propedit.setObject(qprim)
            self.tabwidget.setTabEnabled(2, True)
            
    def untoggleButtons(self):
        self.mainTab.move_button.setChecked(False)
        self.mainTab.rotate_button.setChecked(False)
        self.mainTab.scale_button.setChecked(False)
        
    def highlightEntityFromList(self, ent):
        if self.mainTabList.has_key(str(ent.id)):
            tWid = self.mainTabList[str(ent.id)][1]
            tWid.setSelected(True)
        
    def addToList(self, ent):
        if not self.mainTabList.has_key(str(ent.id)):
           tWid = QTreeWidgetItem(self.mainTab.treeWidget)
           id = ent.id
           tWid.setText(0, id)
            
           self.mainTabList[str(id)] = (ent, tWid)
           return True
        return False
    def showName(self, ent):
        """show the id and name of the object. name is sometimes empty it seems. 
        swoot: actually, seems like the name just isn't gotten fast enough or 
        something.. next time you click on the same entity, it has a name."""
            
        name = ent.name
        if name == "":
            name = "n/a"
        self.mainTab.label.text = "%d (name: %s)" % (ent.id, name)
        
    def on_exit(self):
        self.proxywidget.hide()
        uism = r.getUiSceneManager()
        uism.RemoveProxyWidgetFromScene(self.proxywidget)
        uism.RemoveProxyWidgetFromScene(self.propeditwidget)
        
    def objectDeleted(self, ent_id): #XXX not the best way of doing this
        if self.mainTabList.has_key(ent_id):
            id, tWid = self.mainTabList.pop(ent_id)
            tWid.delete()
import rexviewer as r

import PythonQt
from PythonQt.QtGui import QWidget, QTreeWidgetItem, QSizePolicy, QIcon, QHBoxLayout, QVBoxLayout, QComboBox, QDoubleSpinBox, QPixmap, QLabel, QComboBox
from PythonQt.QtUiTools import QUiLoader
from PythonQt.QtCore import QFile, QSize, Qt
import conversions as conv
reload(conv)
import math

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
    "0" : "Texture"
}

class ObjectEditWindow:
    UIFILE = "pymodules/objectedit/editobject.ui"
    
    ICON_OK = "pymodules/objectedit/ok-small.png"
    ICON_CANCEL = "pymodules/objectedit/cancel-small.png" 
    
    def __init__(self, controller):
        self.controller = controller
        loader = QUiLoader()
        uifile = QFile(self.UIFILE)

        ui = loader.load(uifile)
        width = ui.size.width()
        height = ui.size.height()
        
        #if not DEV:
        uism = r.getUiSceneManager()
        #uiprops = r.createUiWidgetProperty(1) # 1 = Qt::Dialog
        #uiprops.name_ = "Object Edit"
        #uiprops.my_size_ = QSize(width, height) #not needed anymore, uimodule reads it
        self.proxywidget = r.createUiProxyWidget(ui)
        self.proxywidget.setWindowTitle("Object Edit")

        if not uism.AddWidgetToScene(self.proxywidget):
            r.logInfo("Adding ProxyWidget failed.")

        uism.AddWidgetToMenu(self.proxywidget, "Object Edit", "", "./data/ui/images/menus/edbutton_OBJED_normal.png")

        self.widget = ui
        self.tabwidget = ui.findChild("QTabWidget", "MainTabWidget")

        # Tabs
        self.mainTab = ui.findChild("QWidget", "MainFrame")
        self.materialTab = ui.findChild("QWidget", "MaterialsTab")
        self.tabwidget.setTabEnabled(1, False)
        self.materialTabFormWidget = self.materialTab.formLayoutWidget
        self.mainTab.label.text = "<none>"

        # Mesh line edit and buttons
        self.meshline = lines.MeshAssetidEditline(controller) 
        self.meshline.name = "meshLineEdit"

        button_ok = self.getButton("Apply", self.ICON_OK, self.meshline, self.meshline.applyAction)
        button_cancel = self.getButton("Cancel", self.ICON_CANCEL, self.meshline, self.meshline.cancelAction)
        
        #box = self.mainTab.findChild("QHBoxLayout", "meshLine")
        box = QHBoxLayout()
        box.setContentsMargins(0,0,0,0)
        box.addWidget(self.meshline)
        box.addWidget(button_ok)
        box.addWidget(button_cancel)
        self.mesh_widget = QWidget()
        self.mesh_widget.setLayout(box)
        
        # Sound line edit and buttons
        self.soundline = lines.SoundAssetidEditline(controller) 
        self.soundline.name = "soundLineEdit"
        soundbutton_ok = self.getButton("Apply", self.ICON_OK, self.soundline, self.soundline.applyAction)
        soundbutton_cancel = self.getButton("Cancel", self.ICON_CANCEL, self.soundline, self.soundline.cancelAction)
        soundRadius = self.getDoubleSpinBox("soundRadius", "Set sound radius", self.soundline)
        soundVolume = self.getDoubleSpinBox("soundVolume", "Set sound volume", self.soundline)
        
        main_box = QVBoxLayout()
        main_box.setContentsMargins(0,0,0,0)
        box_buttons = QHBoxLayout()
        box_buttons.setContentsMargins(0,0,0,0)
        
        # TODO no need for self?
        self.label_radius = QLabel("Radius")
        self.label_radius.setSizePolicy(QSizePolicy.Fixed, QSizePolicy.Preferred)
        self.label_volume = QLabel("Volume")
        self.label_volume.setSizePolicy(QSizePolicy.Fixed, QSizePolicy.Preferred)

        box_buttons.addWidget(self.label_radius)
        box_buttons.addWidget(soundRadius)
        box_buttons.addWidget(self.label_volume)
        box_buttons.addWidget(soundVolume)        
        box_buttons.addWidget(soundbutton_ok)
        box_buttons.addWidget(soundbutton_cancel)

        main_box.addWidget(self.soundline)
        main_box.addLayout(box_buttons)
        self.sound_widget = QWidget()
        self.sound_widget.setLayout(main_box)

        # Animation line edit and buttons
        self.animationline = lines.AnimationAssetidEditline(controller)
        self.animationline.name = "animationLineEdit"
        animation_combobox = self.getCombobox("AnimationName", "Animation Name", self.animationline)
        animationbutton_ok = self.getButton("Apply", self.ICON_OK, self.animationline, self.animationline.applyAction)
        animationbutton_cancel = self.getButton("Cancel", self.ICON_CANCEL, self.animationline, self.animationline.cancelAction)
        animationRate = self.getDoubleSpinBox("animationRate", "Set animation rate", self.animationline)

        animationbox = QVBoxLayout()
        animationbox.setContentsMargins(0,0,0,0)
        self.anim_box_buttons = QHBoxLayout()
        self.anim_box_buttons.name = "AnimBoxButtons"
        self.anim_box_buttons.setContentsMargins(0,0,0,0)

        label_rate = QLabel("Animation Rate")
        label_rate.name = "Animation Rate"
        label_rate.setSizePolicy(QSizePolicy.Fixed, QSizePolicy.Preferred)

        self.anim_box_buttons.addWidget(label_rate)
        self.anim_box_buttons.addWidget(animation_combobox)
        self.anim_box_buttons.addWidget(animationRate)
        self.anim_box_buttons.addWidget(animationbutton_ok)
        self.anim_box_buttons.addWidget(animationbutton_cancel)

        animationbox.addWidget(self.animationline)
        animationbox.addLayout(self.anim_box_buttons)
        self.animation_widget = QWidget()
        self.animation_widget.setLayout(animationbox)

        # Properties, dead code really..
        self.propedit = r.getPropertyEditor()
        self.tabwidget.addTab(self.propedit, "Properties")
        self.tabwidget.setTabEnabled(2, False)

        self.updatingSelection = False
        
        def poschanger(i):
            def pos_at_index(v):
                self.controller.changepos(i, v)
            return pos_at_index
        for i, poswidget in enumerate([self.mainTab.xpos, self.mainTab.ypos, self.mainTab.zpos]):
            poswidget.connect('valueChanged(double)', poschanger(i))

        def rotchanger(i):
            def rot_at_index(v):
                if not self.controller.usingManipulator and not self.updatingSelection:
                    self.controller.changerot(i, (self.mainTab.rot_x.value, self.mainTab.rot_y.value, self.mainTab.rot_z.value))
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

        # mesh buttons
        self.meshline.connect('textEdited(QString)', button_ok.lineValueChanged)
        self.meshline.connect('textEdited(QString)', button_cancel.lineValueChanged)

        # audio buttons
        self.soundline.connect('textEdited(QString)', soundbutton_ok.lineValueChanged)
        self.soundline.connect('textEdited(QString)', soundbutton_cancel.lineValueChanged)
        soundRadius.connect('valueChanged(double)', soundbutton_ok.lineValueChanged)
        soundRadius.connect('valueChanged(double)', soundbutton_cancel.lineValueChanged)
        soundVolume.connect('valueChanged(double)', soundbutton_ok.lineValueChanged)
        soundVolume.connect('valueChanged(double)', soundbutton_cancel.lineValueChanged)

        # animation buttons
        self.animationline.connect('textEdited(QString)', animationbutton_ok.lineValueChanged)
        self.animationline.connect('textEdited(QString)', animationbutton_cancel.lineValueChanged)
        animationRate.connect('valueChanged(double)', animationbutton_ok.lineValueChanged)
        animationRate.connect('valueChanged(double)', animationbutton_cancel.lineValueChanged)
        animation_combobox.connect('currentIndexChanged(int)', animationbutton_ok.lineValueChanged)
        animation_combobox.connect('currentIndexChanged(int)', animationbutton_cancel.lineValueChanged)

        # actions
        self.mainTab.findChild("QPushButton", "newObject").connect('clicked()', self.controller.createObject)
        self.mainTab.findChild("QPushButton", "deleteObject").connect('clicked()', self.controller.deleteObject)
        self.mainTab.findChild("QPushButton", "duplicate").connect('clicked()', self.controller.duplicate)
        
        self.mainTab.findChild("QPushButton", "undo").connect('clicked()', self.controller.undo)
        
        # transforms
        self.mainTab.findChild("QToolButton", "move_button").connect('clicked()', self.manipulator_move)
        self.mainTab.findChild("QToolButton", "scale_button").connect('clicked()', self.manipulator_scale)
        self.mainTab.findChild("QToolButton", "rotate_button").connect('clicked()', self.manipulator_rotate)
        self.mainTab.useLocalTransform.connect('toggled(bool)', self.controller.setUseLocalTransform)

        self.mainTabList = {}
        
        self.currentlySelectedTreeWidgetItem = []

    def update_guivals(self, ent):
        if ent is not None:
            self.update_posvals(ent.placeable.Position)
            self.update_scalevals(ent.placeable.Scale)
            self.update_rotvals(ent.placeable)
            #self.controller.updateSelectionBox(ent) #PositionAndOrientation(ent)
        
    def update_scalevals(self, scale):
        self.mainTab.scalex.setValue(scale.x())
        self.mainTab.scaley.setValue(scale.y())
        self.mainTab.scalez.setValue(scale.z())
        if self.controller.cpp_python_handler != None:
            self.controller.cpp_python_handler.SetScaleValues(scale.x(), scale.y(), scale.z())

    def update_posvals(self, pos):
        self.mainTab.xpos.setValue(pos.x())
        self.mainTab.ypos.setValue(pos.y())
        self.mainTab.zpos.setValue(pos.z())
        if self.controller.cpp_python_handler != None:
            self.controller.cpp_python_handler.SetPosValues(pos.x(), pos.y(), pos.z())

    def update_rotvals(self, placeable):
        # We use now pitch, yaw and roll we get directly from placeable
        # this ensures we don't have to do weird conversions with all
        # potential problems :)
        # TODO: figure out this shift - Looks like we need to give Ogre-style
        # info. Yaw in viewer is around z axis, but placeable gets
        # directly Ogre orientation
        x_val = math.degrees(placeable.Pitch)
        y_val = math.degrees(placeable.Yaw)
        z_val = math.degrees(placeable.Roll)
        self.mainTab.rot_x.setValue(x_val)
        self.mainTab.rot_y.setValue(y_val)
        self.mainTab.rot_z.setValue(z_val)
        if self.controller.cpp_python_handler != None:
            self.controller.cpp_python_handler.SetRotateValues(x_val, y_val, z_val)
    
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
        self.soundline.update_text("")
        self.animationline.update_text("")

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

    def updateAnimation(self, ent):
        combobox = self.animationline.combobox
        combobox.clear()
        self.animationline.update_text('')
        self.animationline.update_animationrate(0.0)
        self.animation_widget.setEnabled(False)
        if not ent:
            return

        try:
            ent.mesh
        except:
            # TODO put in some nice text field what's going on
            return

        self.animation_widget.setEnabled(True)
        self.animationline.update_text(ent.prim.AnimationPackageID)
        self.animationline.update_animationrate(ent.prim.AnimationRate)

        if ent.prim.AnimationPackageID in (u'', '00000000-0000-0000-0000-000000000000'):
            return

        try:
            ac = ent.animationcontroller
        except:
            ent.createComponent('EC_OgreAnimationController')
            ac = ent.animationcontroller
            ac.SetMeshEntity(ent.mesh)

        current_animation = ent.prim.AnimationName
        available_animations = ac.GetAvailableAnimations()
        for anim in available_animations:
            combobox.addItem(anim)

        if current_animation in available_animations:
            idx = combobox.findText(current_animation)
            combobox.setCurrentIndex(idx)

    def updateMaterialTab(self, ent):
        #ent = self.controller.active
        if ent is not None:
            self.clearDialogForm()
            qprim = ent.prim
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
        size = QSize(20, 20)
        button = buttons.PyPushButton()
        icon = QIcon(QPixmap(iconname).scaled(size))
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

    def getDoubleSpinBox(self, name, tooltip, line):
        spinner = QDoubleSpinBox()
        spinner.setValue(0.0)
        spinner.name = name
        spinner.toolTip = tooltip
        spinner.setEnabled(True)
        line.spinners.append(spinner)
        return spinner

    def getCombobox(self, name, tooltip, line):
        combobox = QComboBox()
        combobox.name = name
        combobox.toolTip = tooltip
        combobox.setEnabled(True)
        line.combobox = combobox
        return combobox

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
        else: #activated
            if ent is not None:
                self.controller.changeManipulator(self.controller.MANIPULATE_ROTATE)
                self.mainTab.scale_button.setChecked(False)
                self.mainTab.move_button.setChecked(False)
            else:
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
        self.meshline.update_text(ent.prim.MeshID)
        self.soundline.update_text(ent.prim.SoundID)
        self.soundline.update_soundradius(ent.prim.SoundRadius)
        self.soundline.update_soundvolume(ent.prim.SoundVolume)
        self.updateAnimation(ent)
        self.updateMaterialTab(ent)
        self.updatePropertyEditor(ent)
        self.updatingSelection = True
        self.update_guivals(ent)
        self.updatingSelection = False
        self.controller.soundRuler(ent)

    def updatePropertyEditor(self, ent):
        qprim = ent.prim
        if qprim is not None:
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
            
        name = ent.prim.Name

        if name == "":
            self.mainTab.label.text = "%d" % (ent.id)
        else:
            self.mainTab.label.text = "%d Name: %s" % (ent.id, name)
        
    def on_exit(self):
        self.proxywidget.hide()
        uism = r.getUiSceneManager()
        uism.RemoveWidgetFromMenu(self.proxywidget)
        uism.RemoveWidgetFromScene(self.proxywidget)
        
    def objectDeleted(self, ent_id): #XXX not the best way of doing this
        if self.mainTabList.has_key(ent_id):
            id, tWid = self.mainTabList.pop(ent_id)
            tWid.delete()
            

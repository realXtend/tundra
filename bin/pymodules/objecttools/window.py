import naali
import math

from PythonQt.QtUiTools import QUiLoader
from PythonQt.QtCore import QFile

class ObjectToolsWindow:
    UIFILE = "pymodules/objecttools/ObjectManipulationsWidget.ui"
    
    def __init__(self, controller):
        self.active = True
        self.controller = controller

        loader = QUiLoader()
        loader.setLanguageChangeEnabled(True)
        uifile = QFile(self.UIFILE)
        
        self.widget = loader.load(uifile)

        if self.widget:
            self.widget.setWindowTitle("Edit Tools")
            naali.ui.AddWidgetToScene(self.widget, True, True)
            naali.ui.AddWidgetToMenu(self.widget,"Edit Tools", "View")
        
            
            #self.widget.findChild("QWidget","tab_widget").setVisible(False)
                        
            #self.widget.findChild("QPushButton", "button_new").setVisible(False)
            #self.widget.findChild("QPushButton", "button_clone").setVisible(False)
            #self.widget.findChild("QPushButton", "button_delete").setVisible(False)
            
            #Manipulator buttons
            self.button_freemove = self.widget.findChild("QPushButton", "button_freemove")
            self.button_freemove.connect("clicked()", self.on_freemove_clicked)
            self.button_move = self.widget.findChild("QPushButton", "button_move")
            self.button_move.connect("clicked()", self.on_move_clicked)
            self.button_scale = self.widget.findChild("QPushButton", "button_scale")
            self.button_scale.connect("clicked()", self.on_scale_clicked)
            self.button_rotate = self.widget.findChild("QPushButton", "button_rotate")
            self.button_rotate.connect("clicked()", self.on_rotate_clicked)
            
            #Manipulator frames
            self.pos_frame = self.widget.findChild("QFrame", "pos_frame")
            self.scale_frame = self.widget.findChild("QFrame", "scale_frame")
            self.rotate_frame = self.widget.findChild("QFrame", "rotate_frame")
            
            #Scale spin boxes
            self.scale_x = self.widget.findChild("QDoubleSpinBox", "doubleSpinBox_scale_x")
            self.scale_x.connect("valueChanged(double)", self.on_scale_changed)
            
            self.scale_y = self.widget.findChild("QDoubleSpinBox", "doubleSpinBox_scale_y")
            self.scale_y.connect("valueChanged(double)", self.on_scale_changed)
            
            self.scale_z = self.widget.findChild("QDoubleSpinBox", "doubleSpinBox_scale_z")
            self.scale_z.connect("valueChanged(double)", self.on_scale_changed)
            
            #Position spin boxes
            self.pos_x = self.widget.findChild("QDoubleSpinBox", "doubleSpinBox_pos_x")
            self.pos_x.connect("valueChanged(double)", self.on_pos_changed)
            
            self.pos_y = self.widget.findChild("QDoubleSpinBox", "doubleSpinBox_pos_y")
            self.pos_y.connect("valueChanged(double)", self.on_pos_changed)
            
            self.pos_z = self.widget.findChild("QDoubleSpinBox", "doubleSpinBox_pos_z")
            self.pos_z.connect("valueChanged(double)", self.on_pos_changed)
            
            #Rotation sliders
            self.rot_x = self.widget.findChild("QSlider", "slider_rotate_x")
            self.rot_x.connect("valueChanged(int)", self.on_rotation_x)
            
            self.rot_y = self.widget.findChild("QSlider", "slider_rotate_y")
            self.rot_y.connect("valueChanged(int)", self.on_rotation_y)
            
            self.rot_z = self.widget.findChild("QSlider", "slider_rotate_z")
            self.rot_z.connect("valueChanged(int)", self.on_rotation_z)
            
            self.deselected()
        
        
    #Methods called by controller        
    def selected(self, ent):
        if ent is not None:
            if hasattr(ent, "placeable"):
				self.update_gui(ent)           
				self.button_scale.setEnabled(True)
				self.button_move.setEnabled(True)
				self.button_rotate.setEnabled(True)
				self.button_freemove.setEnabled(True)
        
    def deselected(self):
        self.button_scale.setEnabled(False)
        self.button_move.setEnabled(False)
        self.button_rotate.setEnabled(False)
        self.button_freemove.setEnabled(False)       
        self.rotate_frame.setVisible(False)
        self.scale_frame.setVisible(False)
        self.pos_frame.setVisible(False)
        
    def changeManipulator(self, manipulator):
        self.rotate_frame.setVisible(False)
        self.scale_frame.setVisible(False)
        self.pos_frame.setVisible(False)
        
        if manipulator == self.controller.MANIPULATE_MOVE:
            self.pos_frame.setVisible(True)
        elif manipulator == self.controller.MANIPULATE_ROTATE:
            self.rotate_frame.setVisible(True)
        elif manipulator == self.controller.MANIPULATE_SCALE:
            self.scale_frame.setVisible(True)

    def update_gui(self,ent):
        self.active = False
        
        self.pos_x.setValue(ent.placeable.Position.x())
        self.pos_y.setValue(ent.placeable.Position.y())
        self.pos_z.setValue(ent.placeable.Position.z())
        
        self.scale_x.setValue(ent.placeable.Scale.x())
        self.scale_y.setValue(ent.placeable.Scale.y())
        self.scale_z.setValue(ent.placeable.Scale.z())
        
        rot_x = math.degrees(ent.placeable.Pitch)
        rot_y = math.degrees(ent.placeable.Yaw)
        rot_z = math.degrees(ent.placeable.Roll)
        if rot_x < 0:
            rot_x += 360
        if rot_y < 0:
            rot_y += 360
        if rot_z < 0:
            rot_z += 360
                
        self.rot_x.setValue(rot_x)
        self.rot_y.setValue(rot_y)
        self.rot_z.setValue(rot_z)
                
        self.widget.findChild("QLabel", "label_rotate_x_value").setText(str(rot_x))
        self.widget.findChild("QLabel", "label_rotate_y_value").setText(str(rot_y))
        self.widget.findChild("QLabel", "label_rotate_z_value").setText(str(rot_z))
        
        self.active = True
    
    #Methods called by widget
    def on_freemove_clicked(self):
        if self.controller.manipulator == self.controller.manipulators[self.controller.MANIPULATE_FREEMOVE]:
            self.controller.changeManipulator(self.controller.MANIPULATE_SELECT)
        else:
            self.controller.changeManipulator(self.controller.MANIPULATE_FREEMOVE)
	
    def on_move_clicked(self):
		if self.controller.manipulator == self.controller.manipulators[self.controller.MANIPULATE_MOVE]:
			self.controller.changeManipulator(self.controller.MANIPULATE_SELECT)
		else:
			self.controller.changeManipulator(self.controller.MANIPULATE_MOVE)
        
    def on_scale_clicked(self):    
		if self.controller.manipulator == self.controller.manipulators[self.controller.MANIPULATE_SCALE]:
			self.controller.changeManipulator(self.controller.MANIPULATE_SELECT)
		else:
			self.controller.changeManipulator(self.controller.MANIPULATE_SCALE)
        
    def on_rotate_clicked(self):        
		if self.controller.manipulator == self.controller.manipulators[self.controller.MANIPULATE_ROTATE]:
			self.controller.changeManipulator(self.controller.MANIPULATE_SELECT)
		else:
			self.controller.changeManipulator(self.controller.MANIPULATE_ROTATE)
    
    def on_scale_changed(self, value):
        if self.active:
            self.controller.changescale_cpp(self.scale_x.value, self.scale_y.value, self.scale_z.value)
   
    def on_pos_changed(self, value):
        if self.active:
            self.controller.changepos_cpp(self.pos_x.value, self.pos_y.value, self.pos_z.value);
    
    def on_rotation_x(self, value):
        if self.active:            
            self.controller.changerot_cpp(value, self.rot_y.value, self.rot_z.value)
    
    def on_rotation_y(self, value):
        if self.active:
            self.controller.changerot_cpp(self.rot_x.value, value, self.rot_z.value)
        
    def on_rotation_z(self, value):
        if self.active:
            self.controller.changerot_cpp(self.rot_x.value, self.rot_y.value, value)  
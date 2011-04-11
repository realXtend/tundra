from PythonQt.QtGui import QAction, QIcon, QActionGroup

import naali

class ObjectToolsToolbar:

    def __init__(self, controller):
        self.controller = controller
        
        self.edit_toolbar = naali.ui.GetExternalToolbar("EditToolBar")
        if self.edit_toolbar:
            self.edit_toolbar.hide() 
            self.action_group = QActionGroup(self.edit_toolbar)
            
            self.action_select = QAction(QIcon("./media/icons/select.png"),"Select", self.edit_toolbar)
            self.edit_toolbar.addAction(self.action_select)
            self.action_select.setEnabled(False)
            self.action_select.connect("triggered()", self.on_action_select)
            self.action_select.setCheckable(True)
            self.action_group.addAction(self.action_select)
            self.action_select.setChecked(True)
            
            self.action_freemove = QAction(QIcon("./media/icons/freemove.png"),"Freemove", self.edit_toolbar)
            self.edit_toolbar.addAction(self.action_freemove)
            self.action_freemove.setEnabled(False)
            self.action_freemove.connect("triggered()", self.on_action_freemove)
            self.action_freemove.setCheckable(True)
            self.action_group.addAction(self.action_freemove)
            
            self.action_move = QAction(QIcon("./media/icons/move.png"),"Move", self.edit_toolbar)
            self.edit_toolbar.addAction(self.action_move)
            self.action_move.setEnabled(False)
            self.action_move.connect("triggered()", self.on_action_move)
            self.action_move.setCheckable(True)
            self.action_group.addAction(self.action_move)
            
            self.action_scale = QAction(QIcon("./media/icons/scale.png"),"Scale", self.edit_toolbar)
            self.edit_toolbar.addAction(self.action_scale)
            self.action_scale.setEnabled(False)
            self.action_scale.connect("triggered()", self.on_action_scale)
            self.action_scale.setCheckable(True)
            self.action_group.addAction(self.action_scale)
            
            self.action_rotate = QAction(QIcon("./media/icons/rotate.png"),"Rotate", self.edit_toolbar)
            self.edit_toolbar.addAction(self.action_rotate)
            self.action_rotate.setEnabled(False)
            self.action_rotate.connect("triggered()", self.on_action_rotate)
            self.action_rotate.setCheckable(True)
            self.action_group.addAction(self.action_rotate)
            
            self.action_edit = QAction(QIcon("./media/icons/build.png"),"Edit Mode", self.edit_toolbar)
            self.edit_toolbar.addAction(self.action_edit)
            self.action_edit.setCheckable(True)
            self.action_edit.connect("toggled(bool)", self.on_action_edit)
            
    def selected(self,ent):
        if ent is not None:
            if self.edit_toolbar:
                self.action_move.setEnabled(True)
                self.action_rotate.setEnabled(True)
                self.action_scale.setEnabled(True)
                self.action_freemove.setEnabled(True)
                self.action_select.setEnabled(True)
            
    def deselected(self):
        if self.edit_toolbar:
            self.action_move.setEnabled(False)
            self.action_rotate.setEnabled(False)
            self.action_scale.setEnabled(False)    
            self.action_freemove.setEnabled(False)            
            self.action_select.setEnabled(False)        
            
    def on_action_move(self):
        if len(self.controller.sels)>0:
            if self.controller.manipulator == self.controller.manipulators[self.controller.MANIPULATE_MOVE]:
                self.controller.changeManipulator(self.controller.MANIPULATE_SELECT)
            else:
                self.controller.changeManipulator(self.controller.MANIPULATE_MOVE)
            
    def on_action_freemove(self):
        if len(self.controller.sels)>0:
            if self.controller.manipulator == self.controller.manipulators[self.controller.MANIPULATE_FREEMOVE]:
                self.controller.changeManipulator(self.controller.MANIPULATE_SELECT)
            else:
                self.controller.changeManipulator(self.controller.MANIPULATE_FREEMOVE)
            
    def on_action_rotate(self):
        if len(self.controller.sels)>0:
            if self.controller.manipulator == self.controller.manipulators[self.controller.MANIPULATE_ROTATE]:
                self.controller.changeManipulator(self.controller.MANIPULATE_SELECT)
            else:
                self.controller.changeManipulator(self.controller.MANIPULATE_ROTATE)
    
    def on_action_scale(self):
        if len(self.controller.sels)>0:
            if self.controller.manipulator == self.controller.manipulators[self.controller.MANIPULATE_SCALE]:
                self.controller.changeManipulator(self.controller.MANIPULATE_SELECT)
            else:
                self.controller.changeManipulator(self.controller.MANIPULATE_SCALE)
            
    def on_action_select(self):
        if len(self.controller.sels)>0:
            self.controller.changeManipulator(self.controller.MANIPULATE_SELECT)
            
    def on_action_edit(self, checked):
        self.controller.toggleEditing(checked)
        self.action_select.setChecked(True)
    
    def toogleEditing(self, editing):
        if editing != self.action_edit.isChecked():
            self.action_edit.setChecked(editing)
            
    def update_toolbar(self):
        if self.controller.manipulator == self.controller.manipulators[self.controller.MANIPULATE_MOVE]:
            self.action_move.setChecked(True)
        elif self.controller.manipulator == self.controller.manipulators[self.controller.MANIPULATE_FREEMOVE]:
            self.action_freemove.setChecked(True)
        elif self.controller.manipulator == self.controller.manipulators[self.controller.MANIPULATE_ROTATE]:
            self.action_rotate.setChecked(True)
        elif self.controller.manipulator == self.controller.manipulators[self.controller.MANIPULATE_SCALE]:
            self.action_scale.setChecked(True)
        else:
            self.action_select.setChecked(True)
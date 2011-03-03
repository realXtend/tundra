from PythonQt.QtGui import QAction, QIcon

import naali

class ObjectToolsToolbar:
   
    def __init__(self, controller):
        self.controller = controller
        
        self.edit_toolbar = naali.ui.GetExternalToolbar("EditToolBar")
        if self.edit_toolbar:
            self.edit_toolbar.setEnabled(False)
            self.edit_toolbar.hide() 
            
            self.action_move = QAction(QIcon("./media/icons/move.png"),"Move", self.edit_toolbar)
            self.edit_toolbar.addAction(self.action_move)
            self.action_move.connect("triggered()", self.on_action_move)
            
            self.action_rotate = QAction(QIcon("./media/icons/rotate.png"),"Rotate", self.edit_toolbar)
            self.edit_toolbar.addAction(self.action_rotate)
            self.action_rotate.connect("triggered()", self.on_action_rotate)
            
            self.action_scale = QAction(QIcon("./media/icons/scale.png"),"Scale", self.edit_toolbar)
            self.edit_toolbar.addAction(self.action_scale)
            self.action_scale.connect("triggered()", self.on_action_scale)
            
    def selected(self):
        if self.edit_toolbar:
            self.edit_toolbar.setEnabled(True)
        
    def deselected(self):
        if self.edit_toolbar:
            self.edit_toolbar.setEnabled(False)            
            
    def on_action_move(self):
        if len(self.controller.sels)>0:
            self.controller.changeManipulator(self.controller.MANIPULATE_MOVE)
        
    def on_action_rotate(self):
        if len(self.controller.sels)>0:
            self.controller.changeManipulator(self.controller.MANIPULATE_ROTATE)
    
    def on_action_scale(self):
        if len(self.controller.sels)>0:
            self.controller.changeManipulator(self.controller.MANIPULATE_SCALE)
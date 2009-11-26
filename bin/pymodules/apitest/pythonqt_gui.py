import rexviewer as r
import PythonQt
from PythonQt.QtGui import QGroupBox, QVBoxLayout, QPushButton, QLineEdit, QInputDialog 

from circuits import Component

INTERNAL = 1
EXTERNAL = 0

class TestGui(Component):
    def __init__(self):
        Component.__init__(self)
        #loader = QUiLoader() 
        self.canvas = r.createCanvas(EXTERNAL)

        #group = QGroupBox()
        #box = QVBoxLayout(group)
        #button = QPushButton(group)
        #box.addWidget(button)
        #group.show()
        #self.canvas.AddWidget(group)

        lineedit = QLineEdit()
        lineedit.show()
        self.canvas.AddWidget(lineedit)

        self.canvas.Show()

        #QInputDialog.getText(None, "Mesh asset UUID", "Please give mesh asset UUID", QLineEdit.Normal, "")

        r.logDebug("TestGui inited")
                
    def on_exit(self):
        r.logDebug("TestGui exiting...")
        
        #qtmod = r.getQtModule()
        #qtmod.DeleteCanvas(self.canvas)

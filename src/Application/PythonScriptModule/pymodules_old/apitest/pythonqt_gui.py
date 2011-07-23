import rexviewer as r
import naali
import PythonQt
from PythonQt import QtGui
from PythonQt.QtGui import QLineEdit, QGroupBox, QVBoxLayout, QPushButton

from circuits import Component

#INTERNAL = 1
#EXTERNAL = 0

#print dir(PythonQt)
#UiWidgetProperties = PythonQt.__dict__['UiServices::UiWidgetProperties']

class TestGui(Component):
    def __init__(self):
        Component.__init__(self)
        #loader = QUiLoader()
        #uism = r.getUiSceneManager() #self.canvas = r.createCanvas(EXTERNAL)

        group = QGroupBox()
        box = QVBoxLayout(group)

        button1 = QPushButton("Button 1", group)
        box.addWidget(button1)

        button2 = QPushButton("Button 2", group)
        box.addWidget(button2)

        slider = QtGui.QSlider(PythonQt.QtCore.Qt.Horizontal)
        box.addWidget(slider)
        slider.connect('valueChanged(int)', self.changed)

        #group.show()

        lineedit = QtGui.QLineEdit()
        box.addWidget(lineedit)
        lineedit.connect('textChanged(QString)', self.changed)

        self.widget = group

        #naali proxywidget boilerplate
        GUINAME = "PythonQt Test"
        uism = naali.ui
        self.proxywidget = r.createUiProxyWidget(self.widget)
        self.proxywidget.setWindowTitle(GUINAME)
        if not uism.AddWidgetToScene(self.proxywidget):
            print "Adding the ProxyWidget to the bar failed."
        # TODO: Due to core UI API refactor AddWidgetToMenu doesn't exist anymore.
        #uism.AddWidgetToMenu(self.proxywidget, GUINAME, "Developer Tools")

        #self.canvas.Show()

        #QInputDialog.getText(None, "Mesh asset UUID", "Please give mesh asset UUID", QLineEdit.Normal, "")

        r.logInfo("TestGui inited.")

    def changed(self, val):
        r.logInfo("TestGui widget value changed: %s" % str(val))
                
    def on_exit(self):
        r.logInfo("TestGui starting exit...")
        
        #qtmod = r.getQtModule()
        #qtmod.DeleteCanvas(self.canvas)

        uism = naali.ui
        uism.RemoveProxyWidgetFromScene(self.proxywidget)
        r.logInfo("TestGui proxywidget removed from scene.")

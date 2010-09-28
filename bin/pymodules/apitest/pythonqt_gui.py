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

        uism = naali.ui
        uiprops = r.createUiWidgetProperty(1)
        uiprops.widget_name_ = "Test Widget"
        #uiprops.my_size_ = QSize(width, height)
        #self.proxywidget = uism.AddWidgetToScene(ui, uiprops)
        self.proxywidget = r.createUiProxyWidget(self.widget, uiprops)
        ##print widget, dir(widget)
        if not uism.AddProxyWidget(self.proxywidget):
            print "Adding the ProxyWidget to the bar failed."

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

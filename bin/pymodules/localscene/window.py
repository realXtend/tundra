import rexviewer as r
import PythonQt
from PythonQt.QtGui import QTreeWidgetItem, QSizePolicy, QIcon, QHBoxLayout, QComboBox, QFileDialog
from PythonQt.QtUiTools import QUiLoader
from PythonQt.QtCore import QFile, QSize




class ToolBarWindow:

    def __init__(self, uistring):
        loader = QUiLoader()
        uifile = QFile(uistring)
        ui = loader.load(uifile)
        self.gui = ui
        width = ui.size.width()
        height = ui.size.height()
        
        uism = r.getUiSceneManager()
        uiprops = r.createUiWidgetProperty(1) #1 is ModuleWidget, shown at toolbar
        #uiprops.widget_name_ = "Object Edit"
        uiprops.widget_name_ = "Local Scene"
        #uiprops.my_size_ = QSize(width, height) #not needed anymore, uimodule reads it
        self.proxywidget = r.createUiProxyWidget(ui, uiprops)

        if not uism.AddProxyWidget(self.proxywidget):
            print "Adding the ProxyWidget to the bar failed."
        pass

    def on_exit(self):
        print "on_exit"
        self.proxywidget.hide()
        uism = r.getUiSceneManager()
        uism.RemoveProxyWidgetFromScene(self.proxywidget)
        
        
class LocalSceneWindow(ToolBarWindow):
    UIFILE = "pymodules/localscene/localscene.ui"
    
    def __init__(self, controller):
        self.controller = controller
        ToolBarWindow.__init__(self, "pymodules/localscene/localscene.ui")
        
        self.widget = self.gui        
        self.xpos = self.gui.findChild("QDoubleSpinBox", "xpos")
        self.ypos = self.gui.findChild("QDoubleSpinBox", "ypos")
        self.zpos = self.gui.findChild("QDoubleSpinBox", "zpos")
        
        self.xscale = self.gui.findChild("QDoubleSpinBox", "xscale")
        self.yscale = self.gui.findChild("QDoubleSpinBox", "yscale")
        self.zscale = self.gui.findChild("QDoubleSpinBox", "zscale")
        
        self.btnLoad = self.gui.findChild("QPushButton", "pushButtonLoad")
        self.btnUnload = self.gui.findChild("QPushButton", "pushButtonUnload")
        self.btnPublish = self.gui.findChild("QPushButton", "pushButtonPublish")
        
        self.chkBoxFlipZY = self.gui.findChild("QCheckBox", "checkBoxFlipZY")
        
        self.btnLoad.connect("clicked(bool)", self.btnLoadClicked)
        self.btnUnload.connect("clicked(bool)", self.btnUnloadClicked)
        self.btnPublish.connect("clicked(bool)", self.btnPublishClicked)
        
        self.xpos.connect("valueChanged(double)", self.spinBoxXPosValueChanged)
        self.ypos.connect("valueChanged(double)", self.spinBoxYPosValueChanged)
        self.zpos.connect("valueChanged(double)", self.spinBoxZPosValueChanged)

        self.xscale.connect("valueChanged(double)", self.spinBoxXScaleValueChanged)
        self.yscale.connect("valueChanged(double)", self.spinBoxYScaleValueChanged)
        self.zscale.connect("valueChanged(double)", self.spinBoxZScaleValueChanged)
        
        self.chkBoxFlipZY.connect("toggled(bool)", self.checkBoxZYToggled)
        
        # print self.gui
        
        pass
        
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

    def btnLoadClicked(self, args):
        print "load clicked"
        #filename=QFileDialog.getOpenFileName("", "*.scene", self, "FileDialog")
        filename=QFileDialog.getOpenFileName(self.widget, "FileDialog")
        self.controller.loadScene(filename)

    def btnUnloadClicked(self, args):
        print "unload clicked"
        self.controller.unloadScene()
        
    def btnPublishClicked(self, args):
        print "publish clicked"
        
    def spinBoxXPosValueChanged(self, double):
        self.controller.setxpos(double)
    def spinBoxYPosValueChanged(self, double):
        self.controller.setypos(double)
    def spinBoxZPosValueChanged(self, double):
        self.controller.setzpos(double)

    def spinBoxXScaleValueChanged(self, double):
        self.controller.setxscale(double)
    def spinBoxYScaleValueChanged(self, double):
        self.controller.setyscale(double)
    def spinBoxZScaleValueChanged(self, double):
        self.controller.setzscale(double)

    def checkBoxZYToggled(self, enabled):
        self.controller.checkBoxZYToggled(enabled)
        
    
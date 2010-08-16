import rexviewer as r
import PythonQt
from PythonQt.QtGui import QTreeWidgetItem, QSizePolicy, QIcon, QHBoxLayout, QComboBox, QFileDialog, QMessageBox, QWidget

from PythonQt.QtUiTools import QUiLoader
from PythonQt.QtCore import QFile, QSize, SIGNAL

import Queue

#class ToolBarWindow(circuits.Thread):
class ToolBarWindow():

    def __init__(self, uistring, queue, endApplication, controller):
        self.controller = controller
        loader = QUiLoader()
        uifile = QFile(uistring)
        ui = loader.load(uifile)
        self.gui = ui
        width = ui.size.width()
        height = ui.size.height()
        
        uism = r.getUiSceneManager()
        #uiprops = r.createUiWidgetProperty(1) # 1 = Qt::Dialog

        #uiprops.my_size_ = QSize(width, height) #not needed anymore, uimodule reads it
        self.proxywidget = r.createUiProxyWidget(ui)
        self.proxywidget.setWindowTitle("Local Scene")

        if not uism.AddWidgetToScene(self.proxywidget):
            r.logInfo("Adding the ProxyWidget to the bar failed.")

        uism.AddWidgetToMenu(self.proxywidget, "Local Scene", "Server Tools", "./data/ui/images/menus/edbutton_LSCENE_normal.png")

        self.inputQueue = queue
        self.endApplication = endApplication
        pass
        
    def on_exit(self):
        try:
            # end incoming loop
            self.controller.isrunning = 0
            self.inputQueue.put('__end__', '')
            
            self.proxywidget.hide()
            uism = r.getUiSceneManager()
            uism.RemoveWidgetFromScene(self.proxywidget)
            return True
        except:
            r.logInfo("LocalSceneWindow failure:")
            traceback.print_exc()
            return False

    # for receiving input message events from other threads
    def processIncoming(self):
        #print "processIncoming"
        while(self.inputQueue.qsize()):
            try:
                title, msg = self.inputQueue.get(0)
                if(title=="__end__"):
                    self.controller.isrunning = 0
                    return
                if(title=="__unload__"):
                    self.controller.unloadScene()
                    return
                self.displayMessage(title, msg)
            except Queue.Empty:
                pass

    def displayMessage(self, title, msg):
        print "displayMessage"
        QMessageBox.information(None, title, msg)
                
        
class LocalSceneWindow(ToolBarWindow, QWidget):
    UIFILE = "pymodules/localscene/localscene.ui"
    
    def __init__(self, controller, queue, endApplication):
        #PythonQt.PythonQtInstanceWrapper.__new__()
        ToolBarWindow.__init__(self, "pymodules/localscene/localscene.ui", queue, endApplication, controller)
        
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
        self.btnSave = self.gui.findChild("QPushButton", "pushButtonSave")
        
        self.chkBoxFlipZY = self.gui.findChild("QCheckBox", "checkBoxFlipZY")
        self.checkBoxHighlight = self.gui.findChild("QCheckBox", "checkBoxHighlight")
        self.checkBoxLockScale = self.gui.findChild("QCheckBox", "checkBoxLockScale")
        
        self.btnLoad.connect("clicked(bool)", self.btnLoadClicked)
        self.btnUnload.connect("clicked(bool)", self.btnUnloadClicked)
        self.btnPublish.connect("clicked(bool)", self.btnPublishClicked)
        self.btnSave.connect("clicked(bool)", self.btnSaveClicked)
        #self.btnSave.connect("clicked(bool)", self.threadTest)
        
        self.xpos.connect("valueChanged(double)", self.spinBoxXPosValueChanged)
        self.ypos.connect("valueChanged(double)", self.spinBoxYPosValueChanged)
        self.zpos.connect("valueChanged(double)", self.spinBoxZPosValueChanged)

        self.xscale.connect("valueChanged(double)", self.spinBoxXScaleValueChanged)
        self.yscale.connect("valueChanged(double)", self.spinBoxYScaleValueChanged)
        self.zscale.connect("valueChanged(double)", self.spinBoxZScaleValueChanged)
        
        self.chkBoxFlipZY.connect("toggled(bool)", self.checkBoxZYToggled)
        self.checkBoxHighlight.connect("toggled(bool)", self.checkBoxHighlightToggled)
        self.checkBoxLockScale.connect("toggled(bool)", self.checkBoxLockScaleToggled)
        
        self.sizeLock = False
        self.filename = ""
        pass

    def threadTest(self):
        self.controller.closeThread()
                        
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
        self.filename=QFileDialog.getOpenFileName(self.widget, "FileDialog")
        self.controller.loadScene(self.filename)

    def btnUnloadClicked(self, args):
        self.controller.unloadScene()
        
    def btnPublishClicked(self, args):
        #self.controller.publishScene(self.filename)
        self.controller.startUpload(self.filename)
        
        
    def btnSaveClicked(self, args):
        self.controller.saveScene(self.filename)
        
        
    def spinBoxXPosValueChanged(self, double):
        self.controller.setxpos(double)
    def spinBoxYPosValueChanged(self, double):
        self.controller.setypos(double)
    def spinBoxZPosValueChanged(self, double):
        self.controller.setzpos(double)

    def spinBoxXScaleValueChanged(self, double):
        if(self.sizeLock):
            self.controller.setxscale(double)
            self.controller.setyscale(double)
            self.controller.setzscale(double)
            self.yscale.setValue(double)
            self.zscale.setValue(double)
        else:
            self.controller.setxscale(double)
            
    def spinBoxYScaleValueChanged(self, double):
        if(self.sizeLock):
            self.controller.setxscale(double)
            self.controller.setyscale(double)
            self.controller.setzscale(double)
            self.xscale.setValue(double)
            self.zscale.setValue(double)
        else:
            self.controller.setyscale(double)
    
    def spinBoxZScaleValueChanged(self, double):
        if(self.sizeLock):
            self.controller.setxscale(double)
            self.controller.setyscale(double)
            self.controller.setzscale(double)
            self.xscale.setValue(double)
            self.yscale.setValue(double)
        else:
            self.controller.setzscale(double)

    def checkBoxZYToggled(self, enabled):
        self.controller.checkBoxZYToggled(enabled)
        
    def checkBoxHighlightToggled(self, enabled):
        self.controller.checkBoxHighlightToggled(enabled)
        
    def checkBoxLockScaleToggled(self, enabled):
        self.sizeLock = enabled
        

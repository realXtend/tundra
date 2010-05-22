import rexviewer as r

import PythonQt
from PythonQt.QtGui import QTreeWidgetItem, QSizePolicy, QIcon, QHBoxLayout, QComboBox
from PythonQt.QtUiTools import QUiLoader
from PythonQt.QtCore import QFile, QSize

class RegionFrameWindow:
    UIFILE = "pymodules/regionframe/framemanager.ui"

    def __init__(self, controller):
        self.controller = controller
        loader = QUiLoader()
        uifile = QFile(self.UIFILE)

        ui = loader.load(uifile)
        width = ui.size.width()
        height = ui.size.height()

        uism = r.getUiSceneManager()
        uiprops = r.createUiWidgetProperty(1) #1 is ModuleWidget, shown at toolbar
        uiprops.widget_name_ = "Region Frame"

        base_url = "./data/ui/images/menus/"
        uiprops.SetMenuNodeIconNormal(base_url + "edbutton_OBJED_normal.png")
        uiprops.SetMenuNodeIconHover(base_url + "edbutton_OBJED_hover.png")
        uiprops.SetMenuNodeIconPressed(base_url + "edbutton_OBJED_click.png")
        
        self.proxywidget = r.createUiProxyWidget(ui, uiprops)        

        if not uism.AddProxyWidget(self.proxywidget):
            print "Adding the ProxyWidget to the bar failed."
        
        self.widget = ui

        ui.findChild("QPushButton", "listButton").connect('clicked()', self.controller.listFrames)
        ui.findChild("QPushButton", "clearButton").connect('clicked()', self.controller.clearFrames)
        ui.findChild("QPushButton", "saveButton").connect('clicked()', self.controller.saveFrame)
        ui.findChild("QPushButton", "unloadButton").connect('clicked()', self.controller.unloadFrame)
        ui.findChild("QPushButton", "loadButton").connect('clicked()', self.controller.loadFrame)

        listWidget = ui.findChild("QListWidget", "listWidget")
        listWidget.connect('itemActivated(QListWidgetItem)', self.ItemActivated)
        listWidget.connect('clicked(QModelIndex)', self.ItemActivated)

    def PopulateList(self, frames):
        listWidget = self.widget.findChild("QListWidget", "listWidget")
        listWidget.clear()
        i = 0
        for frame in frames:
            listWidget.insertItem(i, frame)
            i += 1

    def GetSaveFileName(self):
        valueInEditBox = self.widget.findChild("QLineEdit", "framenameEdit").text
        #this value should have atleast the name of last selected frame
        #print "returned filename:", valueInEditBox
        return valueInEditBox

    def ItemActivated(self, item=None):
        #print "itemActivated singal recived"
        current = self.widget.findChild("QListWidget", "listWidget").currentItem()

        text = ""
        if current != None:
            text = current.text()
        self.widget.findChild("QLineEdit", "framenameEdit").text = text

    def on_exit(self):
        self.proxywidget.hide()
        uism = r.getUiSceneManager()
        uism.RemoveProxyWidgetFromScene(self.proxywidget)

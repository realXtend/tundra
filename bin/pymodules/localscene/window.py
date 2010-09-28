import rexviewer as r
import naali
import PythonQt
from PythonQt.QtGui import QTreeWidgetItem, QSizePolicy, QIcon, QHBoxLayout, QComboBox, QFileDialog, QMessageBox, QWidget, QTableWidgetItem

from PythonQt.QtUiTools import QUiLoader
from PythonQt.QtCore import QFile, QSize, SIGNAL

import Queue
import sys
import traceback

from xml.dom.minidom import getDOMImplementation


class ToolBarWindow():

    def __init__(self, uistring, queue, endApplication, controller):
        self.controller = controller
        loader = QUiLoader()
        uifile = QFile(uistring)
        ui = loader.load(uifile)
        self.gui = ui
        width = ui.size.width()
        height = ui.size.height()
        
        uism = naali.ui
        
        # uiprops = r.createUiWidgetProperty(1) #1 is ModuleWidget, shown at toolbar
        # uiprops.SetMenuGroup(2) #2 is server tools group
        # uiprops.widget_name_ = "Local Scene"
        
        #uiprops.my_size_ = QSize(width, height) #not needed anymore, uimodule reads it
        #self.proxywidget = r.createUiProxyWidget(ui, uiprops)
        
        self.proxywidget = r.createUiProxyWidget(ui)

        #if not uism.AddProxyWidget(self.proxywidget):
        if not uism.AddWidgetToScene(self.proxywidget):
            r.logInfo("Adding the ProxyWidget to the bar failed.")

        uism.AddWidgetToMenu(self.proxywidget, "Local Scene", "Server Tools", "./data/ui/images/menus/edbutton_LSCENE_normal.png")
            
        self.inputQueue = queue
        self.endApplication = endApplication
        
        self.tableitems = []
        
        pass
        
    def on_exit(self):
        try:
            # end incoming loop
            self.controller.isrunning = 0
            self.inputQueue.put('__end__', '')
            
            self.proxywidget.hide()
            uism = naali.ui
            uism.RemoveWidgetFromMenu(self.proxywidget)
            uism.RemoveWidgetFromScene(self.proxywidget)
            return True
        except:
            r.logInfo("LocalSceneWindow failure:")
            traceback.print_exc()
            return False

    
    def processIncoming(self):
        """ for receiving input message events from other threads """
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
        QMessageBox.information(None, title, msg)
                


class LocalSceneWindow(ToolBarWindow, QWidget):
    UIFILE = "pymodules/localscene/localscene.ui"
    
    def __init__(self, controller, queue, endApplication):
    
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
        
        # server end scene editing
        self.btnLoadServerSceneList = self.gui.findChild("QPushButton", "pushButtonLoadServerSceneList")
        self.btnLoadServerScene = self.gui.findChild("QPushButton", "pushButtonLoadServerScene")
        self.btnUnloadServerScene = self.gui.findChild("QPushButton", "pushButtonUnloadServerScene")
        self.btnDeleteServerScene = self.gui.findChild("QPushButton", "pushButtonDeleteServerScene")
        self.btnDeleteActiveScene = self.gui.findChild("QPushButton", "pushButtonDeleteActiveScene")
        
        self.tableWidgetServerScenes = self.gui.findChild("QTableWidget","tableWidgetServerScenes")
        
        self.listViewScenesRegions = self.gui.findChild("QListWidget","listViewScenesRegions")        
        
        self.lineEditRegionName = self.gui.findChild("QLineEdit", "lineEditRegionName")
        self.lineEditPublishName = self.gui.findChild("QLineEdit", "lineEditPublishName")
        self.lineEditLoadRegion = self.gui.findChild("QLineEdit", "lineEditLoadRegion")


        self.progressBar = self.gui.findChild("QProgressBar", "progressBar")
        
        
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

        self.btnLoadServerSceneList.connect("clicked(bool)", self.btnLoadServerSceneListClicked)
        self.btnLoadServerScene.connect("clicked(bool)", self.btnLoadServerSceneClicked)
        self.btnUnloadServerScene.connect("clicked(bool)", self.btnUnloadServerSceneClicked)
        self.btnDeleteServerScene.connect("clicked(bool)", self.btnDeleteServerSceneClicked)
        self.btnDeleteActiveScene.connect("clicked(bool)", self.btnDeleteActiveSceneClicked)
        
        self.tableWidgetServerScenes.connect("itemSelectionChanged()", self.tableWidgetServerScenesItemSelectionChanged)
        
        
        #self.progressBar.setTotalSteps(7)
        self.progressBar.setMinimum(0)
        self.progressBar.setMaximum(7)
        self.progressBar.reset()
        self.progressBar.setValue(0)
        
        self.progressBar.setFormat("Upload progress: inactive %p%")
        
        self.sizeLock = False
        self.filename = ""
        self.serverScenes = {} # key container
        self.serverSceneNames = {}
        
        self.serverSceneRegionLists = {} # key = sceneid, value = list of regions
        self.serverSceneRegionCount = {} # key = sceneid, value = count of regions
        
        self.currentSceneSelectionRegions = []
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
        if(self.filename!=""):
            self.controller.loadScene(self.filename)
        else:
            pass

    def btnUnloadClicked(self, args):
        self.controller.unloadScene()
        
    def btnPublishClicked(self, args):
        #self.controller.publishScene(self.filename)
        regName = self.lineEditRegionName.text
        publishName = self.lineEditPublishName.text
        self.controller.startUpload(self.filename, regName, publishName)
        
        
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

        
# Server side scene handlers
        
    def btnLoadServerSceneListClicked(self, args):
        self.controller.startSceneAction("GetUploadSceneList")
        pass
        
    def btnLoadServerSceneClicked(self, args):
        
        #if self.isThereItemSelected(self.listViewRegionScenes):
        if self.isThereItemSelected(self.tableWidgetServerScenes):
            #self.controller.
            region = self.lineEditLoadRegion.text
            sceneid = self.serverScenes[self.tableWidgetServerScenes.currentRow()]
            data = self.constructLoadSceneData(region, sceneid)
            
            #self.controller.startSceneAction("LoadServerScene", self.serverPassiveScenes[self.listViewRegionScenes.currentRow])
            self.controller.startSceneAction("LoadServerScene", data)
            pass

        pass
        
    def btnUnloadServerSceneClicked(self, args):
        if self.isThereItemSelected(self.tableWidgetServerScenes):
            tablerow = self.tableWidgetServerScenes.currentRow()
            row = self.listViewScenesRegions.currentRow
            param = (self.serverScenes[tablerow], self.currentSceneSelectionRegions[row])
            self.controller.startSceneAction("UnloadServerScene", param)
            pass
        pass
        
    def btnDeleteServerSceneClicked(self, args):
        #self.controller.printOutCurrentCap()
        # if self.isThereItemSelected(self.listViewRegionScenes):
            # print self.serverPassiveScenes[self.listViewRegionScenes.currentRow]
            # self.controller.startSceneAction("DeleteServerScene", self.serverPassiveScenes[self.listViewRegionScenes.currentRow])
            # pass
        pass

    def btnDeleteActiveSceneClicked(self, args):
        if self.isThereTableItemSelected(self.tableWidgetServerScenes):
            self.controller.startSceneAction("DeleteServerScene", self.serverScenes[self.tableWidgetServerScenes.currentRow()])
            pass
        pass
        
    def isThereItemSelected(self, list):
        if list.currentItem() != None:
            return True
        else:
            self.displayMessage('no scene selected', 'Select scene from list for loading/unloading/deleting it.')
            return False

    def isThereTableItemSelected(self, tablelist):
        if tablelist.currentRow() != -1:
            return True
        else:
            self.displayMessage('no scene selected', 'Select scene from table for loading/unloading/deleting it.')
            return False
           
            
    def setServerScenes(self, d):        
        self.tableWidgetServerScenes.clearContents()
        self.serverSceneRegionLists = {}
        self.serverSceneRegionCount = {}		
        #self.tableitems = []
        #self.listViewRegionScenes.clear()
        # need to figure out how to free tableitems
        #del self.tableitems[:]
        while(self.tableWidgetServerScenes.rowCount!=0):
            self.tableWidgetServerScenes.removeRow(0)
        
        uuids = []
        
        if d!=None:
            i = 0
            j = 0
            for k, vals in d.items():
                sceneName, regionName, uuid = vals
                if uuid not in uuids:
                    uuids.append(uuid)
                    self.tableWidgetServerScenes.insertRow (self.tableWidgetServerScenes.rowCount)
                    self.serverScenes[i] = uuid
                    
                    self.serverSceneNames[i] = sceneName
                    sceneNameItem = QTableWidgetItem(sceneName)
                    self.tableitems.append(sceneNameItem)
                    self.tableWidgetServerScenes.setItem(self.tableWidgetServerScenes.rowCount-1, 0, sceneNameItem)
                    i+=1
                
                # store regions
                if(self.serverSceneRegionLists.has_key(uuid)==False):
                    self.serverSceneRegionLists[uuid] = []
                self.serverSceneRegionLists[uuid].append(regionName)

                
                if(self.serverSceneRegionCount.has_key(uuid)==False):
                    if regionName != "":
                        self.serverSceneRegionCount[uuid] = 1
                    else:
                        self.serverSceneRegionCount[uuid] = 0
                elif regionName != "":
                    self.serverSceneRegionCount[uuid]+= 1
                else:
                    self.serverSceneRegionCount[uuid] = 0
                    
                regionCountItem = QTableWidgetItem(str(self.serverSceneRegionCount[uuid]))
                self.tableitems.append(regionCountItem)
                
                row = 0
                for r in self.serverScenes.keys():
                    if self.serverScenes[r]==uuid:
                        row = r
                        break
                self.tableWidgetServerScenes.setItem(row, 1, regionCountItem)
        pass
        
        
    def constructLoadSceneData(self, regionTxt, sceneuuidTxt):
        impl = getDOMImplementation()
        doc = impl.createDocument(None, "loadscenedata", None)
        top_element = doc.documentElement
        region = doc.createElement('region')
        sceneuuid = doc.createElement('sceneuuid')
        top_element.appendChild(region)
        top_element.appendChild(sceneuuid)

        regionName = doc.createTextNode(regionTxt)
        sceneUuid = doc.createTextNode(sceneuuidTxt)

        region.appendChild(regionName)
        sceneuuid.appendChild(sceneUuid)
        return doc.toxml()
        

    def tableWidgetServerScenesItemSelectionChanged(self):
        self.currentSceneSelectionRegions = []
        self.listViewScenesRegions.clear()
        row = self.tableWidgetServerScenes.currentRow()
        if(row!=-1):
            uuid = self.serverScenes[row]
            regions = self.serverSceneRegionLists[uuid]
            for region in regions:
                if(region!=""):
                    self.listViewScenesRegions.addItem(region)
                    self.currentSceneSelectionRegions.append(region)
        pass
        

        
        

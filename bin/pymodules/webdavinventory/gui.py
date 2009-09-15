'''
Created on 8.9.2009

@author: jonnena
'''

import sys
import os

from connection import WebDavClient

import rexviewer as r
import PythonQt

from PythonQt.QtGui import QApplication, QDialog, QTreeWidgetItem, QLineEdit, QFileDialog, QInputDialog, QMessageBox, QIcon, QPixmap
from PythonQt.QtUiTools import QUiLoader
from PythonQt.QtCore import QFile

from circuits import Component

EXTERNAL = 0
INTERNAL = 1
NAME = 0
TYPE = 1

class Application(Component):
    
    def __init__(self, webdavIn, httpclient, connectSuccess):
        Component.__init__(self)
        self.guiMain = None
        self.guiUserInput = None
        self.webdav = webdavIn
        self.httpclient = httpclient
        """ NAALI QMODULE CANVAS """
        self.canvas = r.createCanvas(EXTERNAL)
        """ RUN APP """
        self.runApplication(connectSuccess, "Could not retrieve Inventory URL from World Server")

    def runApplication(self, connectSuccess, message):
        if connectSuccess:
            self.guiMain = GuiMain(self)
            self.guiMain.gui.connect("finished(int)", self.closeHandler)
            self.guiMain.gui.show()
        else:
            self.guiUserInput = UserDefinesSettings(message, self.httpclient.storedHost, self.httpclient.storedIdentityType, self.httpclient.storedIdentity)
            self.guiUserInput.gui.pushButton_Connect.connect("clicked(bool)", self.connectHandler)
            self.guiUserInput.gui.pushButton_Close.connect("clicked(bool)", self.closeHandler)
            self.guiUserInput.gui.show()
    
    def connectHandler(self, bool):
        failed = False
        myHost, authType, authInfo  = self.guiUserInput.getUserInput()
        
        try:
            index = authInfo.index(" ")
            firstName = authInfo[0:index]
            lastName = authInfo[index+1:]
            if (self.httpclient.setupConnection(myHost, authType, None, firstName, lastName) == False):
                failed = True
        except ValueError:
            if (self.httpclient.setupConnection(myHost, authType, authInfo, None, None) == False):
                failed = True
        
        try:
            identityurl, webdavurl = self.httpclient.requestIdentityAndWebDavURL()
        except Exception:
            failed = True
        
        if (failed == False):
            self.webdav = WebDavClient(identityurl, webdavurl)
            try:
                self.webdav.setupConnection()
                self.guiUserInput.gui.close()
                self.runApplication(True, None)
            except Exception:
                self.guiUserInput.gui.close()
                self.runApplication(False, "Could not connect to webdav inventory")
        else:
            self.guiUserInput.gui.close()
            self.runApplication(False, "Could not retrieve Inventory URL from World Server")
            
    def closeHandler(self, input):
        if (self.guiUserInput != None):
            self.guiUserInput.gui.close()
        if (self.guiMain != None):
            self.guiMain.gui.close()
        
class GuiMain(object):

    path = ""
    localTempPath = ""
    
    def __init__(self, parent):
        self.app = parent
        self.UIFILE = "pymodules" + os.sep + "webdavinventory" + os.sep + "gui" + os.sep + "main_gui.ui"
        """ UI FROM FILE """
        loader = QUiLoader()
        file = QFile(self.UIFILE)
        self.gui = loader.load(file)
        self.setBottomContolButtonsEnabled(True)
        self.listRootResources()
        self.connectSignals()

    def connectSignals(self):
        """ Bottom Controls """
        self.gui.pushButton_AddDirectory.connect("clicked(bool)", self.on_pushButton_AddDirectory_clicked)
        self.gui.pushButton_Upload.connect("clicked(bool)", self.on_pushButton_Upload_clicked)
        self.gui.pushButton_Download.connect("clicked(bool)", self.on_pushButton_Download_clicked)
        self.gui.pushButton_Delete.connect("clicked(bool)", self.on_pushButton_Delete_clicked)

        """ TreeView, signals from parent QAbstractItemView """
        self.gui.treeWidget.connect("doubleClicked(QModelIndex)", self.itemDoubleClicked)
        self.gui.treeWidget.connect("clicked(QModelIndex)", self.itemClickedSetSelected)
        
    def listRootResources(self):
        self.setBottomContolButtonsEnabled(True)
        self.path = ""
        """ GET INVENTORY """
        self.gui.treeWidget.clear()
        results = self.app.webdav.listResources(None)        
        if (results != False):
            topLevelItems = []
            """ SET RETULTS TO TREEWIDGET """
            for itemName in results:
                item = QTreeWidgetItem()
                itemType = results[itemName].getResourceType()
                item.setText(NAME, str(itemName))
                if (itemType == "resource"):
                    item.setText(TYPE, "Resource")
                    # FIND OUT HOW TO USE QIcon with QPixmap in pythonqt
                    #item.setIcon(0, QIcon(QPixmap("iconResource.png")))
                else:
                    item.setText(TYPE, "Folder")
                    # FIND OUT HOW TO USE QIcon with QPixmap in pythonqt
                    #item.setIcon(0, QIcon(QPixmap("iconCollection.png")))    
                topLevelItems.append(item)
            self.gui.treeWidget.addTopLevelItems(topLevelItems)
            self.setInfoLabel(None)
        else:
            self.connectionLost()
    
    """ DOWNLOAD """
    """ OWN SLOT """
    def on_pushButton_Download_clicked(self):
        selectedItem = self.gui.treeWidget.currentItem()
        if (selectedItem != False):
            """ GET FOLDER LOCATION  FROM USER"""
            filePath = QFileDialog.getExistingDirectory(self.gui, "Select download destination folder")
            if (filePath != "" ):
                """ SET PATHS ADN DOWNLOAD FILE """
                self.localTempPath = ""
                itemLen = -len(str(selectedItem.text(NAME)))
                self.localTempPath = self.path[0:itemLen]
                if ( self.app.webdav.downloadFile(str(filePath), self.localTempPath, str(selectedItem.text(NAME))) ):
                    pass
                else:
                    self.setInfoLabel("Error occurred while downloading file")     

    """ UPLOAD """
    """ OWN SLOT """
    def on_pushButton_Upload_clicked(self):
        selectedItem = self.gui.treeWidget.currentItem()
        if (selectedItem != False):
            """ GET FILE TO UPLOAD FROM USER """
            filePath = QFileDialog.getOpenFileName(self.gui, "Upload File");
            if (filePath != ""):
                uriParts = filePath.split("/")
                resourceName = str(uriParts[-1])
                """ UPLOAD THE FILE """
                if ( self.app.webdav.uploadFile(str(filePath), self.path, resourceName) ):
                    if (selectedItem.text(TYPE) == "Folder"):
                        self.itemDoubleClicked(selectedItem)
                        selectedItem.setExpanded(True)
                else:
                    self.setInfoLabel("Error occurred while uploading file")
               
    """ ADD DIRECTORY """ 
    """ OWN SLOT """
    def on_pushButton_AddDirectory_clicked(self):
        selectedItem = self.gui.treeWidget.currentItem()
        if (selectedItem != False and selectedItem.text(TYPE) == "Folder"):
            directoryName = QInputDialog.getText(self.gui, "Directory Name", "Give new directory name", QLineEdit.Normal, "")
            if (directoryName != ""):
                if ( self.app.webdav.createDirectory(self.path, str(directoryName)) ):
                    self.itemDoubleClicked(selectedItem)
                    selectedItem.setExpanded(True)
                else:
                    self.setInfoLabel("Error occurred while creating directory to /" + self.path)

    """ DELETE RESOURCE(S) """
    """ OWN SLOT """
    def on_pushButton_Delete_clicked(self):
        self.itemsDeleted = 0
        self.foldersDeleted = 0
        selectedItem = self.gui.treeWidget.currentItem()
        if ( self.deleteTreeWidgetFolderAndChildren(selectedItem, self.path) == False):
            message = "Folders deleted: %d  Items Deleted: %d  But errors occurred while deleting resources" % (self.foldersDeleted, self.itemsDeleted)
            self.setInfoLabel(message)
            return
        else:
            treeRoot = self.gui.treeWidget.invisibleRootItem()
            if (selectedItem.parent() != treeRoot):
                self.gui.treeWidget.setCurrentItem(selectedItem.parent())
                self.itemDoubleClicked(None)
            elif (selectedItem.parent() == treeRoot):
                self.on_pushButton_Connect_clicked()
        
    """ DELETE HELPER FUNCTION """
    def deleteTreeWidgetFolderAndChildren(self, childItem, pathInTree):
        myPath = pathInTree
        """ IF resource THEN delete """
        if (childItem != False and childItem.text(TYPE) == "Resource"):
            itemLen = len( str(childItem.text(NAME)) )
            itemLen -= 2*itemLen + 1
            resourceParentPath = myPath[0: itemLen]
            if ( self.app.webdav.deleteResource(resourceParentPath, str(childItem.text(NAME))) == True ):
                return True
            else:
                return False
        """ IF collection THEN list resources """
        if (childItem != False and childItem.text(TYPE) == "Folder"):
            itemLen = len( str(childItem.text(NAME)) )
            itemLen -= 2*itemLen + 1
            myPath = myPath[0: itemLen]
            if (self.app.webdav.deleteDirectory(myPath, str(childItem.text(NAME))) == True):
                return True
            else:
                return False
                
    """ OWN SLOT """
    def itemDoubleClicked(self, indexModel):
        sourceItem = self.gui.treeWidget.currentItem()
        if (sourceItem.text(TYPE) == "Folder"):
            self.removeChildren(sourceItem)
            self.path = ""
            if (sourceItem.parent() != None):
                self.setPathToItem(sourceItem)
            else:
                self.path = str(sourceItem.text(0)) + "/"
            results = self.app.webdav.listResources(self.path)
            if (results != False):
                childList = []
                for childItemName in results:
                    itemType = results[childItemName].getResourceType()
                    item = QTreeWidgetItem()
                    item.setText(NAME, childItemName)
                    item.setText(TYPE, itemType)
                    if (itemType == "resource"):
                        item.setText(TYPE, "Resource")
                        # FIND OUT HOW TO USE QIcon with QPixmap in pythonqt
                        #item.setIcon(0, QIcon(QPixmap("iconResource.png")))
                    else:
                        item.setText(TYPE, "Folder")
                        # FIND OUT HOW TO USE QIcon with QPixmap in pythonqt
                        #item.setIcon(0, QIcon(QPixmap("iconCollection.png")))
                    childList.append(item)
                sourceItem.addChildren(childList)
                self.setInfoLabel(None);
            else:
                self.connectionLost()
    
    """ OWN SLOT """
    def itemClickedSetSelected(self, indexModel):
        sourceItem = self.gui.treeWidget.currentItem()
        self.path = ""
        self.setPathToItem(sourceItem)
        sourceItem.setSelected(True)
        enabled = True
        if (sourceItem.text(TYPE) == "Resource"):
            enabled = False
            self.path = self.path[0:-1]
            self.gui.pushButton_Download.setEnabled(True)
        else:
            self.gui.pushButton_Download.setEnabled(False)
        self.gui.pushButton_Upload.setEnabled(enabled)
        self.gui.pushButton_AddDirectory.setEnabled(enabled)
        self.setInfoLabel(None)
    
    def getTreeItemByName(self, itemName, parent):
        childCount = parent.childCount()
        index = 0
        while index < childCount:
            child = parent.child(index)
            if (str(child.text(NAME)) == itemName):
                return child
        return None
            
    def getSelectedTreeItem(self):
        selectedItems = self.gui.treeWidget.selectedItems()
        if (selectedItems == []):
            self.setInfoLabel("Error: Select Item First")
            return False
        return selectedItems[0]
    
    def removeChildren(self, parent):
        childCount = parent.childCount()
        index = 0
        while (index < childCount):
            parent.removeChild(parent.child(0))
            index += 1
            
    def setPathToItem(self, thisSourceItem):
        if (thisSourceItem.parent() != None):
            self.setPathToItem(thisSourceItem.parent())
        self.path += str(thisSourceItem.text(0)) + "/"

    def setInfoLabel(self, message):
        if (message == None):
            self.gui.label_Path.setText("Current path: /"+self.path)
        else:
            self.gui.label_Path.setText(message)

    def connectionLost(self):
        self.setInfoLabel("Error: the server is down or not a webdav server")
        self.gui.treeWidget.clear()
        self.setBottomContolButtonsEnabled(False)
        self.gui.hide()
        self.app.runApplication(False, "Connection to server lost")
        self.gui.close()
            
    def setBottomContolButtonsEnabled(self, boolean):
        self.gui.treeWidget.setEnabled(boolean)
        self.gui.pushButton_Upload.setEnabled(boolean)
        self.gui.pushButton_Download.setEnabled(boolean)
        self.gui.pushButton_AddDirectory.setEnabled(boolean)
        self.gui.pushButton_Delete.setEnabled(boolean)

class UserDefinesSettings(object):
    
    def __init__(self, message, host = None, authType = None, identity = None):
        self.UIFILE = "pymodules" + os.sep + "webdavinventory" + os.sep + "gui" + os.sep + "setupconnection_gui.ui"
        self.host = host
        self.authType = authType
        self.identity = identity
        """ UI FROM FILE """
        loader = QUiLoader()
        file = QFile(self.UIFILE)
        self.gui = loader.load(file)
        self.gui.label_StatusInfo.setText(message)
        """ INIT FIELDS IF INPUT DATA GIVEN """
        if self.host != None:
            self.gui.lineEdit_ServerUrl.setText(self.host)
        if self.identity != None:
            self.gui.lineEdit_authIdentity.setText(self.identity)
        if authType != None:
            self.gui.comboBox.setCurrentIndex(authType)

    def getUserInput(self):
        self.gui.hide()
        self.host = str(self.gui.lineEdit_ServerUrl.text)
        self.authType = str(self.gui.comboBox.itemText(self.gui.comboBox.currentIndex))
        self.authIdentity = str(self.gui.lineEdit_authIdentity.text)
        return self.host, self.authType, self.authIdentity

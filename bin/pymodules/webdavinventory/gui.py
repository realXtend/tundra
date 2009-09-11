'''
Created on 8.9.2009

@author: jonnena
'''

import sys
import os

from connection import WebDavClient

from PyQt4 import uic
from PyQt4.QtGui import QApplication, QDialog, QTreeWidgetItem, QLineEdit, QFileDialog, QInputDialog, QMessageBox, QIcon, QPixmap
from PyQt4.QtCore import QObject, pyqtSlot, QFile

NAME = 0
TYPE = 1

class Application(QObject):
    
    def __init__(self, webdavIn, httpclient, connectSuccess):
        self.guiMain = None
        self.guiUserInput = None
        self.webdav = webdavIn
        self.httpclient = httpclient
        self.app = QApplication(sys.argv)
        self.runApplication(connectSuccess)
        sys.exit(self.app.exec_())
        
    def runApplication(self, connectSuccess):
        if connectSuccess:
            self.guiMain = GuiMain(self)
            self.guiMain.finished.connect(self.closeHandler)
            self.guiMain.show()
        else:
            self.guiUserInput = UserDefinesSettings("Could not retrieve Inventory URL from World Server")
            self.guiUserInput.pushButton_Connect.clicked.connect(self.connectHandler)
            self.guiUserInput.pushButton_Close.clicked.connect(self.closeHandler)
            self.guiUserInput.show()
    
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
                self.guiUserInput.close()
                self.runApplication(True)
            except Exception:
                self.guiUserInput.close()
                self.runApplication(False)
        else:
            self.guiUserInput.close()
            self.runApplication(False)
            
    def closeHandler(self, bool):
        self.app.exit()
    
class GuiMain(QDialog):

    path = ""
    localTempPath = ""
    
    def __init__(self, parent):
        self.app = parent
        super(GuiMain, self).__init__()
        uic.loadUi("gui" + os.sep + "main_gui.ui", self)
        self.setBottomContolButtonsEnabled(True)
        self.connectSignals()
        self.listRootResources()

    def connectSignals(self):
        treeView = self.treeWidget
        treeView.itemDoubleClicked.connect(self.itemDoubleClicked)
        treeView.itemClicked.connect(self.itemClickedSetSelected)
        
    def listRootResources(self):
        self.setBottomContolButtonsEnabled(True)
        self.path = ""
        """ GET INVENTORY """
        treeView = self.treeWidget
        treeView.clear()
        results = self.app.webdav.listResources(None)
        if (results != False):
            """ SET RETULTS TO TREEWIDGET """
            for itemName in results:
                item = QTreeWidgetItem()
                itemType = results[itemName].getResourceType()
                item.setText(NAME, itemName)
                item.setText(TYPE, itemType)
                if (itemType == "resource"):
                    item.setText(TYPE, "Resource")
                    item.setIcon(0, QIcon(QPixmap("iconResource.png")))
                else:
                    item.setText(TYPE, "Folder")
                    item.setIcon(0, QIcon(QPixmap("iconCollection.png")))     
                treeView.addTopLevelItem(item)
            self.setInfoLabel(None)
        else:
            self.connectionLost()
    
    """ DOWNLOAD """
    @pyqtSlot()
    def on_pushButton_Download_clicked(self):
        selectedItem = self.getSelectedTreeItem()
        if (selectedItem != False):
            """ GET FOLDER LOCATION  FROM USER"""
            filePath = QFileDialog.getExistingDirectory(self, "Select download destination folder")
            if (filePath != "" ):
                """ SET PATHS ADN DOWNLOAD FILE """
                self.localTempPath = ""
                self.setLocalTempPathPathToItem(selectedItem.parent())
                if ( self.app.webdav.downloadFile(str(filePath), self.localTempPath, str(selectedItem.text(NAME))) ):
                    pass
                else:
                    self.setInfoLabel("Error occurred while downloading file")     

    """ UPLOAD """
    @pyqtSlot()
    def on_pushButton_Upload_clicked(self):
        selectedItem = self.getSelectedTreeItem()
        if (selectedItem != False):
            """ GET FILE TO UPLOAD FROM USER """
            filePath = QFileDialog.getOpenFileName(self, "Upload File");
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
    @pyqtSlot()
    def on_pushButton_AddDirectory_clicked(self):
        selectedItem = self.getSelectedTreeItem()
        if (selectedItem != False and selectedItem.text(TYPE) == "Folder"):
            directoryName, OK = QInputDialog.getText(self, "Directory Name", "Give new directory name", QLineEdit.Normal, "")
            if (directoryName != "" and OK):
                if ( self.app.webdav.createDirectory(self.path, str(directoryName)) ):
                    self.itemDoubleClicked(selectedItem)
                    selectedItem.setExpanded(True)
                else:
                    self.setInfoLabel("Error occurred while creating directory to /" + self.path)

    """ DELETE RESOURCE(S) """
    @pyqtSlot()
    def on_pushButton_Delete_clicked(self):
        self.itemsDeleted = 0
        self.foldersDeleted = 0
        selectedItem = self.getSelectedTreeItem()
        if ( self.deleteTreeWidgetFolderAndChildren(selectedItem, self.path) == False):
            message = "Folders deleted: %d  Items Deleted: %d  But errors occurred while deleting resources" % (self.foldersDeleted, self.itemsDeleted)
            self.setInfoLabel(message)
            return
        else:
            treeRoot = self.treeWidget.invisibleRootItem()
            if (selectedItem.parent() != treeRoot):
                self.itemDoubleClicked(selectedItem.parent())
                selectedItem.parent().setExpanded(True)
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
            if ( self.app.webdav.deleteResource(resourceParentPath, str(childItem.text(NAME))) == False ):
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
    def itemDoubleClicked(self, sourceItem):
        if (sourceItem.text(TYPE) == "Folder"):
            self.removeChildren(sourceItem)
            self.path = ""
            if (sourceItem.parent() != None):
                self.setPathToItem(sourceItem)
            else:
                self.path = str(sourceItem.text(0)) + "/"
            results = self.app.webdav.listResources(self.path)
            if (results != False):
                for childItemName in results:
                    itemType = results[childItemName].getResourceType()
                    item = QTreeWidgetItem()
                    item.setText(NAME, childItemName)
                    item.setText(TYPE, itemType)
                    if (itemType == "resource"):
                        item.setText(TYPE, "Resource")
                        item.setIcon(0, QIcon(QPixmap("iconResource.png")))
                    else:
                        item.setText(TYPE, "Folder")
                        item.setIcon(0, QIcon(QPixmap("iconCollection.png")))
                    sourceItem.addChild(item)
                self.setInfoLabel(None);
            else:
                self.connectionLost()
    
    """ OWN SLOT """
    def itemClickedSetSelected(self, sourceItem):
        self.path = ""
        self.setPathToItem(sourceItem)
        sourceItem.setSelected(True)
        enabled = True
        if (sourceItem.text(TYPE) == "Resource"):
            enabled = False
            self.path = self.path[0:-1]
            self.pushButton_Download.setEnabled(True)
        else:
            self.pushButton_Download.setEnabled(False)
        self.pushButton_Upload.setEnabled(enabled)
        self.pushButton_AddDirectory.setEnabled(enabled)
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
        treeView = self.treeWidget
        selectedItems = treeView.selectedItems()
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

    def setLocalTempPathPathToItem(self, thisSourceItem):
        if (thisSourceItem.parent() != None):
            self.setPathToItem(thisSourceItem.parent())
        self.localTempPath += str(thisSourceItem.text(0)) + "/"

    def setInfoLabel(self, message):
        label = self.label_Path
        if (message == None):
            label.setText("Current path: /"+self.path)
        else:
            label.setText(message)

    def connectionLost(self):
        self.setInfoLabel("Error: the server is down or not a webdav server")
        self.treeWidget.clear()
        self.setBottomContolButtonsEnabled(False)
        
    def setBottomContolButtonsEnabled(self, boolean):
        self.treeWidget.setEnabled(boolean)
        self.pushButton_Upload.setEnabled(boolean)
        self.pushButton_Download.setEnabled(boolean)
        self.pushButton_AddDirectory.setEnabled(boolean)
        self.pushButton_Delete.setEnabled(boolean)

class UserDefinesSettings(QDialog):
        
    def __init__(self, message):
        self.host = None
        self.authType = None
        self.identity = None
        super(UserDefinesSettings, self).__init__()
        uic.loadUi("gui" + os.sep + "setupconnection_gui.ui", self)
        self.label_StatusInfo.setText(message)

    def getUserInput(self):
        self.hide()
        self.host = str(self.lineEdit_ServerUrl.text())
        self.authType = str(self.comboBox.itemText(self.comboBox.currentIndex()))
        self.authIdentity = str(self.lineEdit_authIdentity.text())
        return self.host, self.authType, self.authIdentity

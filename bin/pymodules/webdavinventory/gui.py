'''
Created on 8.9.2009

@author: jonnena
'''

import sys
import os
#import time

from connection import WebDavClient
from webdav.Connection import WebdavError

from PyQt4 import uic
from PyQt4.QtGui import QApplication, QDialog, QTreeWidget, QTreeWidgetItem, QLineEdit, QLabel, QFileDialog, QInputDialog, QMessageBox, QIcon, QPixmap, QTreeWidgetItemIterator, QPushButton
from PyQt4.QtCore import QObject, pyqtSlot, QFile

NAME = 0
TYPE = 1
#MODIFIED = 2

class Application(QObject):
    
    def __init__(self, webdavIn, connectSuccess):
        self.guiMain = None
        self.webdav = None
        self.myIdentity = None
        self.myWebDav = None
        self.app = QApplication(sys.argv)
        
        connectionEstablished = False
        
        while (connectionEstablished == False):
            if connectSuccess:
                connectionEstablished = connectSuccess
                self.webdav = webdavIn
                guiMain = GuiMain(self)
                guiMain.show()
            else:
                guiUserInput = UserDefinesSettings(self, "Could not connect to fetched inventory")
                self.myIdentity, self.myWebDav = guiUserInput.askUserInput()
                testWebDav = WebDavClient(str(self.myIdentity), str(self.myWebDav))
                try:
                    testWebDav.setupConnection()
                    connectionEstablished = True
                except WebdavError:
                    connectionEstablished = False
                    
        sys.exit(self.app.exec_())
    
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
        treeView = self.findChild(QTreeWidget, "treeWidget")
        treeView.itemDoubleClicked.connect(self.itemDoubleClicked)
        treeView.itemClicked.connect(self.itemClickedSetSelected)
        
    def listRootResources(self):
        self.setBottomContolButtonsEnabled(True)
        self.path = ""
        """ GET INVENTORY """
        treeView = self.findChild(QTreeWidget, "treeWidget")
        treeView.clear()
        results = self.app.webdav.listResources(None)
        if (results != False):
            """ SET RETULTS TO TREEWIDGET """
            for itemName in results:
                item = QTreeWidgetItem()
                itemType = results[itemName].getResourceType()
                #dateModified = time.asctime(results[itemName].getLastModified())
                item.setText(NAME, itemName)
                item.setText(TYPE, itemType)
                #item.setText(MODIFIED, dateModified)
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
            self.setInfoLabel("Folders deleted: %d  Items Deleted: %d  But errors occurred while deleting resources") % (self.foldersDeleted, self.itemsDeleted)
        else:
            self.setInfoLabel("Folders deleted: %d  Items Deleted: %d  All delete operations executed successfully") % (self.foldersDeleted, self.itemsDeleted)
        treeRoot = self.findChild(QTreeWidget, "treeWidget").invisibleRootItem()
        if (selectedItem.parent() != treeRoot):
            self.itemDoubleClicked(selectedItem.parent())
            selectedItem.parent().setSelected(True)
        elif (selectedItem.parent() == treeRoot):
            self.on_pushButton_Connect_clicked()
            
        
    """ DELETE HELPER FUNCTION """
    def deleteTreeWidgetFolderAndChildren(self, childItem, pathInTree):
        myPath = pathInTree
        """ IF resource THEN delete """
        if (childItem != False and childItem.text(TYPE) == "Resource"):
            if ( self.app.webdav.deleteResource(self.path, str(childItem.text(NAME))) == False ):
                self.itemsDeleted += 1
                return False
        """ IF collection THEN list resources """
        if (childItem != False and childItem.text(TYPE) == "Folder"):
            childItems = []
            childFolders = []
            resources = self.app.webdav.listResources(myPath)
            if (resources != False):
                """ Get and store all child items and folders """
                for childName in resources:
                    itemType = resources[childName].getResourceType()
                    if (itemType == "resource"):
                        childItems.append(childName)
                    elif (itemType == "collection"):
                        childFolders.append(childName)
                    else:
                        return False
                """ Go through child items and delete them """
                for itemName in childItems:
                    foundItem = self.getTreeItemByName(itemName, childItem)
                    if (foundItem != None):
                        if (self.deleteTreeWidgetFolderAndChildren(foundItem, None) == False):
                            return False
                    else:
                        return False
                """ Go through child folders, call this function recursively, finally delete folder """
                for folderName in childFolders:
                    foundFolder = self.getTreeItemByName(folderName, childItem)
                    if (foundFolder != None):
                        if ( self.deleteTreeWidgetFolderAndChildren(foundFolder, myPath + "/" + folderName) == False ):
                            return False
                        else:
                            self.app.webdav.deleteFolder(myPath, foundFolder)
                            self.foldersDeleted += 1
                    else:
                        return False
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
                    #dateModified = time.asctime(results[childItemName].getLastModified())
                    item = QTreeWidgetItem()
                    item.setText(NAME, childItemName)
                    item.setText(TYPE, itemType)
                    #item.setText(MODIFIED, dateModified)
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
            self.findChild(QPushButton, "pushButton_Download").setEnabled(True)
        else:
            self.findChild(QPushButton, "pushButton_Download").setEnabled(False)
        self.findChild(QPushButton, "pushButton_Upload").setEnabled(enabled)
        self.findChild(QPushButton, "pushButton_AddDirectory").setEnabled(enabled)
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
        treeView = self.findChild(QTreeWidget, "treeWidget")
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
        label = self.findChild(QLabel, "label_Path")
        if (message == None):
            label.setText("Current path: /"+self.path)
        else:
            label.setText(message)

    def connectionLost(self):
        self.setInfoLabel("Error: the server is down or not a webdav server")
        self.findChild(QTreeWidget, "treeWidget").clear()
        self.setBottomContolButtonsEnabled(False)
        
    def setBottomContolButtonsEnabled(self, boolean):
        self.findChild(QTreeWidget, "treeWidget").setEnabled(boolean)
        self.findChild(QPushButton, "pushButton_Upload").setEnabled(boolean)
        self.findChild(QPushButton, "pushButton_Download").setEnabled(boolean)
        self.findChild(QPushButton, "pushButton_AddDirectory").setEnabled(boolean)
        self.findChild(QPushButton, "pushButton_Delete").setEnabled(boolean)

class UserDefinesSettings(QDialog):
    
    identity = False
    webdav = False
    
    def __init__(self, parent, message):
        self.identity = None
        self.webdav = None
        self.app = parent
        super(UserDefinesSettings, self).__init__()
        uic.loadUi("gui" + os.sep + "setupconnection_gui.ui", self)
        self.show()
        
    @pyqtSlot()
    def on_pushButton_Connect_clicked(self):
        self.identity = str(self.findChild(QLineEdit, "lineEdit_ServerUrl").text())
        self.webdav = str(self.findChild(QLineEdit, "lineEdit_User").text())
        self.close()
        
    @pyqtSlot()
    def on_pushButton_Close_clicked(self):
        self.exec_()
        
    def askUserInput(self):
        while self.identity == False and self.webdav == False:
            self.identity, OK = QInputDialog.getText(None, "Identity", "Give identity url", QLineEdit.Normal, "http://")
            if (OK and self.identity != ""):
                self.webdav, OK = QInputDialog.getText(None, "Webdav", "Give webdavurl", QLineEdit.Normal, "http://")
                if (OK and self.webdav  != ""):
                    return self.identity, self.webdav
                else:
                    self.identity = False
                    self.webdav = False
            else:
                self.identity = False
        
        
'''
Created on 8.9.2009

@author: jonnena
'''

import sys
import time

from PyQt4 import uic
from PyQt4.QtGui import QApplication, QDialog, QTreeWidget, QTreeWidgetItem, QLineEdit, QLabel, QFileDialog, QInputDialog, QIcon, QPixmap, QTreeWidgetItemIterator, QPushButton
from PyQt4.QtCore import QObject, pyqtSlot, QFile

NAME = 0
TYPE = 1
MODIFIED = 2

class Application(QObject):
    
    guiMain = None
    webdav = None
    
    def __init__(self, webdavIn):
        print "[Application] Starting Qt Application"
        self.webdav = webdavIn
        self.app = QApplication(sys.argv)
        guiMain = GuiMain(self)
        guiMain.show()
        sys.exit(self.app.exec_())
    
class GuiMain(QDialog):

    path = ""
    localTempPath = ""
    
    def __init__(self, parent):
        print "[GUI] Initilizing GuiMain QDialog"
        self.app = parent
        super(GuiMain, self).__init__()
        uic.loadUi('main_gui.ui', self)
        self.setBottomContolButtonsEnabled(False)
        self.connectSignals()
        
    def connectSignals(self):
        treeView = self.findChild(QTreeWidget, "treeWidget")
        treeView.itemDoubleClicked.connect(self.itemDoubleClicked)
        treeView.itemClicked.connect(self.itemClickedSetSelected)
        
    @pyqtSlot()
    def on_pushButton_Connect_clicked(self):
        self.setBottomContolButtonsEnabled(True)
        self.path = ""
        """ INIT CONNECTION """   
        url = str(self.findChild(QLineEdit, "lineEdit_ServerUrl").text())
        user = str(self.findChild(QLineEdit, "lineEdit_User").text())
        passwd = str(self.findChild(QLineEdit, "lineEdit_Password").text())
        if ( self.app.webdav.setConnection(url, user, passwd) == False ):
            self.connectionLost()
            return
        """ GET INVENTORY """
        treeView = self.findChild(QTreeWidget, "treeWidget")
        treeView.clear()
        results = self.app.webdav.listResources(None)
        if (results != False):
            """ SET RETULTS TO TREEWIDGET """
            for itemName in results:
                item = QTreeWidgetItem()
                itemType = results[itemName].getResourceType()
                dateModified = time.asctime(results[itemName].getLastModified())
                item.setText(NAME, itemName)
                item.setText(TYPE, itemType)
                item.setText(MODIFIED, dateModified)
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
    
    @pyqtSlot()
    def on_pushButton_Delete_clicked(self):
        print "click4"
       
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
                    dateModified = time.asctime(results[childItemName].getLastModified())
                    item = QTreeWidgetItem()
                    item.setText(NAME, childItemName)
                    item.setText(TYPE, itemType)
                    item.setText(MODIFIED, dateModified)
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
            
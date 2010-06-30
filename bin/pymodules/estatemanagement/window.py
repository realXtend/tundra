import rexviewer as r
import PythonQt
from PythonQt.QtUiTools import QUiLoader
from PythonQt.QtGui import QWidget, QMessageBox
from PythonQt.QtCore import QFile
import Queue
import sys
#from PythonQt.QtCore import QStringList

import traceback



class IncomingMessagesHandler():
    
    def __init__(self, queue, endmethod):
        self.queue = queue
        self.endmethod = endmethod
        self.handlers = {}
        
    def on_exit(self):
        try:
            # end incoming loop
            self.endmethod()
            self.queue.put('__end__', 'none')
            
            self.proxywidget.hide()
            uism = r.getUiSceneManager()
            uism.RemoveProxyWidgetFromScene(self.proxywidget)
            return True
        except:
            r.logInfo("LocalSceneWindow failure:")
            traceback.print_exc()
            return False

    def processIncoming(self):
        while(self.queue.qsize()):
            try:
                tag = None
                msg = None
                entry = self.queue.get(0)
                try:
                    tag, msg = entry
                except:
                    r.logInfo("LocalSceneWindow wrong type of incoming input:")
                    r.logInfo(entry)
                    continue
                if(self.handlers.__contains__(tag)):
                    #print "calling", tag
                    self.handlers[tag](msg)
                    continue
                if(tag=="__end__"):
                    #self.controller.isrunning = 0
                    self.endmethod()
                    return
                #print "no handler"
            except Queue.Empty:
                pass

    def addHandler(self, tag, method):
        self.handlers[tag]=method
        

    
    
    
class EstateManagementWindow(QWidget, IncomingMessagesHandler):

    UIFILE = "pymodules/estatemanagement/estatemanagement.ui"
    def __init__(self, controller, queue):
        loader = QUiLoader()
        #uifile = QFile("pymodules/estatemanagement/estatemanagement.ui")
        uifile = QFile(EstateManagementWindow.UIFILE)
        self.gui = loader.load(uifile)
        self.controller = controller
        IncomingMessagesHandler.__init__(self, queue, self.endMethod)        
        
        uism = r.getUiSceneManager()
        uiprops = r.createUiWidgetProperty(1) #1 is ModuleWidget, shown at toolbar
        uiprops.SetMenuGroup(2) #2 is server tools group
        uiprops.widget_name_ = "Estate Management"
        
        self.proxywidget = r.createUiProxyWidget(self.gui, uiprops)
        
        if not uism.AddProxyWidget(self.proxywidget):
            r.logInfo("Adding the ProxyWidget to the bar failed.")
        
        self.btnLoadEstate = self.gui.findChild("QPushButton", "btnLoadEstate")
        self.listWEI = self.gui.findChild("QListWidget","listWidgetEstateInfo")
        self.listWRU = self.gui.findChild("QListWidget","listWidgetRegionUsers")
        self.listWEB = self.gui.findChild("QListWidget","listWidgetBanList")
        self.listWAL = self.gui.findChild("QListWidget","listWidgetAccessList")
        self.listWEM = self.gui.findChild("QListWidget","listWidgetEstateManagers")
        self.listWSU = self.gui.findChild("QListWidget","listWidgetSavedUsers")
        
        self.labelEstateInfo = self.gui.findChild("QLabel","labelEstateInfo")
        self.tableEstates = self.gui.findChild("QListWidget","tableWidgetEstates")        
        self.chkEstateAccess = self.gui.findChild("QCheckBox","checkBoxEstateAccess")

        self.lineCurrentEstate = self.gui.findChild("QLineEdit", "lineEditCurrentEstate")
        self.listWidgetEstates = self.gui.findChild("QListWidget", "listWidgetEstates")
        self.btnSetEstate = self.gui.findChild("QPushButton", "pushButtonSetEstate")
         

        self.chkSunFixed = self.gui.findChild("QCheckBox", "checkBoxSunFixed");
        # PublicAllowed
        # ExternallyVisible);
        self.chkAllowVoice = self.gui.findChild("QCheckBox", "checkBoxAllowVoice")
        self.chkAllowDirectTeleport = self.gui.findChild("QCheckBox", "checkBoxAllowDirectTeleport")
        self.chkDenyAnonymous = self.gui.findChild("QCheckBox", "checkBoxDenyAnonymous")
        self.chkDenyIdentified = self.gui.findChild("QCheckBox", "checkBoxDenyIdentified")
        self.chkDenyTransacted = self.gui.findChild("QCheckBox", "checkBoxDenyTransacted")
        self.chkAbuseEmailToEstateOwner = self.gui.findChild("QCheckBox", "checkBoxAbuseEmailToEstateOwner")
        self.chkBlockDwell = self.gui.findChild("QCheckBox", "checkBoxBlockDwell")
        self.chkEstateSkipScripts = self.gui.findChild("QCheckBox", "checkBoxEstateSkipScripts")
        self.chkResetHomeOnTeleport = self.gui.findChild("QCheckBox", "checkBoxResetHomeOnTeleport")
        self.chkTaxFree = self.gui.findChild("QCheckBox", "checkBoxTaxFree")
        self.chkDenyAgeUnverified = self.gui.findChild("QCheckBox", "checkBoxDenyAgeUnverified")

        self.btnLoadEstate.connect("clicked(bool)", self.btnLoadEstateClicked)
        
        self.btnToBan = self.gui.findChild("QPushButton", "pushButtonToBan")
        self.btnToAccess = self.gui.findChild("QPushButton", "pushButtonToAccess")
        self.btnToManagers = self.gui.findChild("QPushButton", "pushButtonToManagers")
        self.btnSaveUserInfo = self.gui.findChild("QPushButton", "pushButtonSaveUserInfo")
        
        self.btnRemoveBan = self.gui.findChild("QPushButton", "pushButtonRemoveBan")
        self.btnRemoveAccess = self.gui.findChild("QPushButton", "pushButtonRemoveAccess")
        self.btnRemoveManagers = self.gui.findChild("QPushButton", "pushButtonRemoveManager")        
        self.btnRemoveSaved = self.gui.findChild("QPushButton", "pushButtonRemoveSaved")
        
        self.btnSavedToAccess = self.gui.findChild("QPushButton", "pushButtonSavedToAccess")
        self.btnSavedToBan = self.gui.findChild("QPushButton", "pushButtonSavedToBan")
        self.btnSavedToManagers = self.gui.findChild("QPushButton", "pushButtonSavedToManagers")
        
        self.btnKick = self.gui.findChild("QPushButton", "pushButtonKick")
        self.btnRequestGodLikePowers = self.gui.findChild("QPushButton", "pushButtonRequestGodLikePowers")
        self.lineKickMessage = self.gui.findChild("QLineEdit","lineEditKickMessage")
        
        
        self.btnToBan.connect("clicked(bool)", self.btnToBanClicked)
        self.btnToAccess.connect("clicked(bool)", self.btnToAccessClicked)
        self.btnToManagers.connect("clicked(bool)", self.btnToManagersClicked)
        self.btnSaveUserInfo.connect("clicked(bool)", self.btnSaveUserInfoClicked)

        self.btnRemoveBan.connect("clicked(bool)", self.btnRemoveBanClicked)
        self.btnRemoveAccess.connect("clicked(bool)", self.btnRemoveAccessClicked)
        self.btnRemoveManagers.connect("clicked(bool)", self.btnRemoveManagersClicked)
        
        self.btnRemoveSaved.connect("clicked(bool)", self.btnRemoveSavedClicked)

        self.btnSavedToAccess.connect("clicked(bool)", self.btnSavedToAccessClicked)
        self.btnSavedToBan.connect("clicked(bool)", self.btnSavedToBanClicked)
        self.btnSavedToManagers.connect("clicked(bool)", self.btnSavedToManagersClicked)
        
        self.btnRequestGodLikePowers.connect("clicked(bool)", self.btnRequestGodLikePowersClicked)
        self.btnKick.connect("clicked(bool)", self.btnKickClicked)
        self.btnSetEstate.connect("clicked(bool)", self.btnSetEstateClicked)
        
        self.chkEstateAccess.connect("toggled(bool)", self.chkEstateAccessToggled)
        
        
        # add incoming event handlers
        self.addHandler('EstateUpdate', self.setEstateInfo)
        self.addHandler('setEstateManagers', self.setEstateManagers)
        self.addHandler('setEstateBans', self.setEstateBans)
        self.addHandler('setEsteteAccessList', self.setEsteteAccessList)
        self.addHandler('noCapability', self.noCapabilityInfoMessage)
        self.addHandler('estateSetting', self.handleEstateAccessSetting)
        self.addHandler('failedEstateSetting', self.handleEstateAccessFailure)
        self.addHandler('unknownResponse', self.handleEstateAccessFailure)
        self.addHandler('malformedResp', self.handleEstateAccessFailure)        
        self.addHandler('currentEstateIdAndName', self.handleCurrentEstateIdAndName)        
        self.addHandler('estates', self.handleEstates)
        self.addHandler('displayerror', self.handleDisplayError)
        
        
    def endMethod(self):
        pass
        
    def on_exit(self):
        try:
            # end incoming loop
            self.controller.isrunning = 0
            
            self.proxywidget.hide()
            uism = r.getUiSceneManager()
            uism.RemoveProxyWidgetFromScene(self.proxywidget)
            return True
        except:
            r.logInfo("Estate Window failure:")
            traceback.print_exc()
            return False
        
    def btnLoadEstateClicked(self):
        self.controller.loadEstate()
        pass
        
    def btnToBanClicked(self):
        if self.isThereItemSelected(self.listWRU):
            self.controller.toBan(self.listWRU.currentRow)
        
    def btnToAccessClicked(self):
        if self.isThereItemSelected(self.listWRU):
            self.controller.toAccess(self.listWRU.currentRow)
            
    def btnToManagersClicked(self):
        if self.isThereItemSelected(self.listWRU):
            self.controller.toManagers(self.listWRU.currentRow)
        
    def btnSaveUserInfoClicked(self):
        if self.isThereItemSelected(self.listWRU):
            self.controller.saveUserInfo(self.listWRU.currentRow)
        
    def btnRemoveBanClicked(self):
        if self.isThereItemSelected(self.listWEB):
            self.controller.removeBan(self.listWEB.currentRow)
        
    def btnRemoveAccessClicked(self):
        if self.isThereItemSelected(self.listWAL):
            self.controller.removeAccess(self.listWAL.currentRow)
        
    def btnRemoveManagersClicked(self):
        if self.isThereItemSelected(self.listWEM):
            self.controller.removeManager(self.listWEM.currentRow)
        
    def btnKickClicked(self):
        if self.isThereItemSelected(self.listWRU):
            self.controller.kickUser(self.listWRU.currentRow)
        

    def btnSavedToAccessClicked(self):
        success, uuid, name = self.itemUuidAndNameFromList(self.listWSU)
        if success:
            #uuid, name = self.takeItemUuidAndName()
            self.controller.fromSavedToAccess(uuid, name)
        
    def btnSavedToBanClicked(self):
        success, uuid, name = self.itemUuidAndNameFromList(self.listWSU)
        if success:
            #uuid, name = self.takeItemUuidAndName()        
            self.controller.fromSavedToBan(uuid, name)
        
    def btnSavedToManagersClicked(self):
        success, uuid, name = self.itemUuidAndNameFromList(self.listWSU)
        if success:
            #uuid, name = self.takeItemUuidAndName()
            self.controller.fromSavedToManagers(uuid, name)
        
    def btnSetEstateClicked(self):
        estateData = self.listWidgetEstates.currentItem().text()
        self.controller.estatesettings.setRegionEstate(estateData)
        pass
        
    def chkEstateAccessToggled(self, checked):
        self.controller.setEstateAccessMode(checked)
        pass
        
    def isThereItemSelected(self, list):
        if list.currentItem() != None:
            return True
        else:
            self.displayMessage('no user selected', 'Select user from list for adding it.')
            return False
        
    def itemUuidAndNameFromList(self, list):
        item = list.currentItem()
        if(item!=None):
            txt = item.text()
            return True, txt.split('|')[1], txt.split('|')[0]
            #return txt.split('|')[0], txt.split('|')[1]
        else:
            self.displayMessage('no user selected', 'Select user from list for adding it.')
            return False, "", ""

        
    def setSavedUsers(self, lines):
        self.listWSU.clear()
        i = 0
        #len = lines.__len__()
        while(i<lines.__len__()):
            line = lines[i].replace('\n','')
            self.listWSU.addItem(line)
            i=i+1
        pass
        
    def btnRemoveSavedClicked(self):
        #item = self.listWSU.currentItem()
        row = self.listWSU.currentRow
        item = self.listWSU.takeItem(row)
        txt = item.text()
        self.controller.removeSaved(txt)
        pass
        
    def btnRequestGodLikePowersClicked(self):
        self.controller.requestGodLikePowers()
        
    def btnKickClicked(self):
        message = self.lineKickMessage.text
        self.controller.sendKick(self.listWRU.currentRow, message)
        
    def setEstateInfo(self, paramlist):
        estateName, estateOwner, estateID, estateFlags, sunPosition, parentEstate, covenant, str1160895077, str1, abuseEmail = paramlist
        self.listWEI.clear()
        self.listWEI.addItem("Name: " + estateName)        
        self.listWEI.addItem("estateOwner: " + estateOwner)
        self.listWEI.addItem("estateID: " + estateID)
        self.listWEI.addItem("estateFlags: " + estateFlags)
        self.listWEI.addItem("sunPosition: " + sunPosition)
        self.listWEI.addItem("parentEstate: " + parentEstate)
        self.listWEI.addItem("covenant: " + covenant)
        self.listWEI.addItem("str1160895077: " + str1160895077)
        self.listWEI.addItem("str1: " + str1)
        self.listWEI.addItem("abuseEmail: " + abuseEmail)
        self.handleEstateFlags(estateFlags)
        pass
        
    def setEstateManagers(self, dict):        
        self.listWEM.clear()
        for key, val in dict.items():
            self.listWEM.addItem(val + "|" + key)
        pass
    def setEstateBans(self, dict):
        self.listWEB.clear()
        for key, val in dict.items():
            self.listWEB.addItem(val + "|" + key)
        pass
    def setEsteteAccessList(self, dict):
        self.listWAL.clear()
        for key, val in dict.items():
            self.listWAL.addItem(val + "|" + key)
        pass

    def setRegionUsers(self, ents):
        self.listWRU.clear()
        for e in ents:
            displaystring = e.opensimpresence.QGetFullName() + "|" + e.opensimpresence.QGetUUIDString()
            self.listWRU.addItem(displaystring)
            #e.opensimpresence.QGetUUIDString()
            #print e.opensimpresence.QGetFullName()
        pass
        
    def displayMessage(self, title, msg):
        QMessageBox.information(None, title, msg)

    def noCapabilityInfoMessage(self, message):
        self.displayMessage('No capability', message)
        
    def handleEstateAccessSetting(self, value):
        if (value[0].upper()=='T'):
            if(self.chkEstateAccess.isChecked()==False):
                self.chkEstateAccess.toggle()
        else:
            if(self.chkEstateAccess.isChecked()==True):
                self.chkEstateAccess.toggle()
                
    def handleEstateAccessFailure(self, message):
        self.displayMessage('something went wrong', message)
        
    def setCheckBox(self, chekbox, checked):
        if(checked):
            if(chekbox.isChecked()==False):
                chekbox.toggle()
        else:
            if(chekbox.isChecked()==True):
                chekbox.toggle()
        
    def handleEstateFlags(self, flags):
        intFlags = int(flags)
        publicAccessOn = 131072|32768 # = 163840
        if intFlags & publicAccessOn != 0:
            #public access on
            self.handleEstateAccessSetting('True')
        else:
            #public access off
            self.handleEstateAccessSetting('False')
        pass

        setVal = intFlags & 16 != 0
        self.setCheckBox(self.chkSunFixed,setVal) #SunFixed
        setVal = intFlags & 268435456 != 0
        self.setCheckBox(self.chkAllowVoice,setVal)
        setVal = intFlags & 1048576 != 0
        self.setCheckBox(self.chkAllowDirectTeleport,setVal)
        setVal = intFlags & 8388608 != 0
        self.setCheckBox(self.chkDenyAnonymous,setVal)
        setVal = intFlags & 16777216 != 0
        self.setCheckBox(self.chkDenyIdentified,setVal)
        setVal = intFlags & 33554432 != 0
        self.setCheckBox(self.chkDenyTransacted,setVal)
        setVal = intFlags & 134217728 != 0
        self.setCheckBox(self.chkAbuseEmailToEstateOwner,setVal)
        setVal = intFlags & 262144 != 0
        self.setCheckBox(self.chkBlockDwell,setVal)
        setVal = intFlags & 2097152 != 0
        self.setCheckBox(self.chkEstateSkipScripts,setVal)
        setVal = intFlags & 8 != 0
        self.setCheckBox(self.chkResetHomeOnTeleport,setVal)
        setVal = intFlags & 32 != 0
        self.setCheckBox(self.chkTaxFree,setVal)
        setVal = intFlags & 1073741824 != 0
        self.setCheckBox(self.chkDenyAgeUnverified,setVal)
        
    def handleCurrentEstateIdAndName(self, currentEstateIdAndName):
        self.lineCurrentEstate.setText(currentEstateIdAndName)

    def handleEstates(self, estates):
        self.listWidgetEstates.clear()
        for e in estates:
            self.listWidgetEstates.addItem(e)

    def handleDisplayError(self, message):
        QMessageBox.information(None, "Error", message)
        pass
            
    def setNoCapInfo(self):
        self.labelEstateInfo.setText("Estate info: (no caps, \nyou will probably not be able to \nmodify access rights) \n check you are logged in \n as addministrator or owner")
        
    def resetCapInfo(self):
        self.labelEstateInfo.setText("Estate info:")    



"""
Module for estate management, attach to login event, ask estate info, show window if user is owner of the estate
"""

import rexviewer as r
import naali
from circuits import Component

import window
from window import EstateManagementWindow

import estatesettings
from estatesettings import EstateSettings


import Queue
import PythonQt
import threading
import os
import sys #for error reporting when calling naali to get folder fails

import binascii


class UIEventManagement:

    def __init__(self, window):
        self.window = window
        self.timer = PythonQt.QtCore.QTimer()
        PythonQt.QtCore.QObject.connect(self.timer,
                       PythonQt.QtCore.SIGNAL("timeout()"),
                       self.periodicCall)
        self.timer.start(1000)
        self.isrunning = True

    def periodicCall(self):
        self.window.processIncoming()
        if not self.isrunning:
            self.timer.stop()
        pass
        
    


class EstateManagement(Component):

    ESTATE_DATA_FOLDER = "estatedata"
    SAVED_USERS_FILE = "savedusers.txt"
    
    def __init__(self):
        Component.__init__(self)
        self.worldstream = r.getServerConnection()
        self.queue = Queue.Queue()
        self.window = EstateManagementWindow(self, self.queue)
        self.uievents = UIEventManagement(self.window) 
        self.rexlogic = r.getRexLogic()
        self.appdatafolder = ""
        try:
            self.appdatafolder = r.getApplicationDataDirectory()
        except:
            r.logInfo("Unexpected error:", sys.exc_info()[0])

        self.rexlogic.connect("OnIncomingEstateOwnerMessage(QVariantList)", self.onEstateOwnerMessage)
        self.ents = []
        self.managers = {}
        self.banned = {}
        self.accesslist = {}
        self.publicestate = True
        self.nameToUuidMapCache = {}
        self.savedusers = {}
        
        self.estatesettings = EstateSettings(self.worldstream, self)

        
    def on_exit(self):
        r.logInfo("EstateManagement exiting...")
        self.rexlogic.disconnect("OnIncomingEstateOwnerMessage(QVariantList)", self.onEstateOwnerMessage)
        self.uievents.isrunning = False
        self.window.on_exit()
        r.logInfo("EstateManagement done exiting...")
        
    def on_hide(self, shown):
        pass
        
    def update(self, time):
        pass
        
    def on_sceneadded(self, name):
        r.logInfo("Estate management scene added.")
        # self.loadEstate() # cant get caps at this time, should have some on_login_region_finished event
        pass    

    def on_logout(self, id):
        r.logInfo("Estate management Logout.")
        
    def on_login(self):
        r.logInfo("Estate management on login.")
        pass
        
    def tryGetNameFromSceneEntities(self, uuid):
        for e in self.ents:
            if(e.opensimpresence.QGetUUIDString()==uuid):
                return e.opensimpresence.QGetFullName()
        return "unknown"        
        
    def tryGetNameFromNameToUuidMapCache(self, uuid):
        for key, val in self.nameToUuidMapCache.items():
            if key==uuid:
                return self.nameToUuidMapCache.pop(key)
        return "unknown"

    def tryGetNameFromSavedUsers(self, uuid):
        for key, val in self.savedusers.items():
            
            if val==uuid:
                return key
                #return d.pop(key)
        return "unknown"
        
    def loadEstate(self):
        s = naali.getScene("World")
        ids = s.GetEntityIdsWithComponent("EC_OpenSimPresence")
        self.ents = [r.getEntity(id) for id in ids]
        self.window.setRegionUsers(self.ents)
        #self.worldstream.SendEstateInfoRequestpacket() #!!
        self.worldstream.SendEstateOwnerMessage("getinfo", ())        
        self.updateSavedUsers()
        #self.updateSavedUsersTab()
        #test if rights to modify access
        cap = self.worldstream.GetCapability('EstateRegionSettingsModification')
        if cap==None or cap=="":
            self.window.setNoCapInfo()
        else:
            self.estatesettings.fetchEstates()
            self.window.resetCapInfo()
        pass
        
    def toBan(self, pos):
        self.sendEstateaccessdelta(pos, 64)
        
    def toAccess(self, pos):
        self.sendEstateaccessdelta(pos, 4)
        
    def toManagers(self, pos):
        self.sendEstateaccessdelta(pos, 256)

    def removeBan(self, pos):
        self.sendEstateaccessdeltaRemove(pos, 128, self.banned)
        pass
        
    def removeAccess(self, pos):
        self.sendEstateaccessdeltaRemove(pos, 8, self.accesslist)
        pass
       
    def removeManager(self, pos):
        self.sendEstateaccessdeltaRemove(pos, 512, self.managers)
        pass
            
    def kickUser(self, pos):
        pass
                
    def sendEstateaccessdelta(self, pos, estateAccessType):
        if(self.ents.__len__()>pos):
            e = self.ents[pos]
            uuid = e.opensimpresence.QGetUUIDString()
            #if ((estateAccessType & 256) != 0) // Manager add
            method = "estateaccessdelta"
            self.worldstream.SendEstateOwnerMessage(method, ("", estateAccessType, uuid))        
        
    def sendEstateaccessdeltaRemove(self, pos, estateAccessType, dict):
        if(dict.__len__()>pos):
            #if ((estateAccessType & 256) != 0) // Manager add
            uuid = dict.keys()[pos]
            method = "estateaccessdelta"
            self.worldstream.SendEstateOwnerMessage(method, ("", estateAccessType, uuid))        
        
        
    def requestGodLikePowers(self):
        self.worldstream.SendRequestGodlikePowersPacket(True)
        pass
        
    def sendKick(self, pos, message):
        if(self.ents.__len__()>pos):
            e = self.ents[pos]
            uuid = e.opensimpresence.QGetUUIDString()
            self.worldstream.SendGodKickUserPacket(uuid, message)
        pass
        
    def saveUserInfo(self, pos):
        if(self.ents.__len__()>pos):
            e = self.ents[pos]
            info = e.opensimpresence.QGetFullName() + "|" + e.opensimpresence.QGetUUIDString() + "\n"
            
            filepath = self.appdatafolder + os.sep + EstateManagement.ESTATE_DATA_FOLDER + os.sep + EstateManagement.SAVED_USERS_FILE
            if(os.access(self.appdatafolder + os.sep + EstateManagement.ESTATE_DATA_FOLDER, os.F_OK)==False):
                # create folder
                os.mkdir(self.appdatafolder + os.sep + EstateManagement.ESTATE_DATA_FOLDER)
            
            f = open(filepath, 'a')
            f.write(info)
            f.close()
            self.updateSavedUsers()
            #self.updateSavedUsersTab()
        pass
        
    def updateSavedUsers(self):
    #def updateSavedUsersTab(self):
        filepath = self.appdatafolder + os.sep + EstateManagement.ESTATE_DATA_FOLDER + os.sep + EstateManagement.SAVED_USERS_FILE
        if(os.access(filepath, os.F_OK)):
            f = open(filepath, 'r')
            lines = f.readlines()
            self.window.setSavedUsers(lines)
            i = 0
            while(i<lines.__len__()):
                line = lines[i].replace('\n','')
                self.savedusers[line.split('|')[0]] = line.split('|')[1]
                i=i+1
            f.close()
        pass        

    def removeSaved(self, line):
        filepath = self.appdatafolder + os.sep + EstateManagement.ESTATE_DATA_FOLDER + os.sep + EstateManagement.SAVED_USERS_FILE
        newlines = []
        if(os.access(filepath, os.F_OK)):
            f = open(filepath, 'r')
            lines = f.readlines()
            f.close()
            
            #os.remove(filepath) # need to remove old data
            
            for sline in lines:
                if sline.strip() == line.strip(): #skip
                    pass
                else:
                    newlines.append(sline)
            
            f = open(filepath, 'w')
            f.writelines(newlines)
            f.close()
        pass
        
    def fromSavedToAccess(self, uuid, name):
        self.nameToUuidMapCache[uuid]=name
        self.worldstream.SendEstateOwnerMessage("estateaccessdelta", ("", 4, uuid))        
        pass
        
    def fromSavedToBan(self, uuid, name):
        self.nameToUuidMapCache[uuid]=name
        self.worldstream.SendEstateOwnerMessage("estateaccessdelta", ("", 64, uuid))        
        pass
        
    def fromSavedToManagers(self, uuid, name):
        self.nameToUuidMapCache[uuid]=name
        self.worldstream.SendEstateOwnerMessage("estateaccessdelta", ("", 256, uuid))        
        pass

    def setEstateAccessMode(self, mode):
        self.estatesettings.setAccessMode(mode)
        
        
    def onEstateOwnerMessage(self, args):        
        method = args[3]
        if(method=='estateupdateinfo'):
            self.handleEstateUpdateMessage(args)
        if(method=='setaccess'):
            self.handleSetAccess(args)
        pass
        
                
    def handleEstateUpdateMessage(self, args):    
        agentid, sessionid, transactionid, method, invoice, paramlist = args
        len = paramlist.__len__()
        if(len!=10):
            r.logInfo('malformed estateupdateinfo')
            return
        self.queue.put(('EstateUpdate', paramlist))
        
    def handleSetAccess(self, args):
        paramlist = args[5]
        code = int(paramlist[1])
        if(code==1): # AccessOptions = 1,
            self.handleAccessOptions(args)
        if(code==2): # AllowedGroups = 2,
            self.handleAllowedGroups(args)
        if(code==4): # EstateBans = 4,
            self.handleEstateBans(args)
        if(code==8): # EstateManagers = 8
            self.handleEstateManagers(args)
        
    def handleAccessOptions(self, args):
        self.processIncomingList(args, 'setEsteteAccessList', self.accesslist)
        pass
        
    def handleAllowedGroups(self, args):
        pass
            
    def handleEstateBans(self, args):
        self.processIncomingList(args, 'setEstateBans', self.banned)
            
    def handleEstateManagers(self, args):
        self.processIncomingList(args, 'setEstateManagers', self.managers)        
        
    def processIncomingList(self, args, uiUpdateCommand, updateList):
        updateList.clear()
        agentid, sessionid, transactionid, method, invoice, paramlist = args
        if(paramlist.__len__()>6):
            for i in range(6, paramlist.__len__()):
                # decode for uuids one by one
                uuid = self.uuidFromHex(paramlist[i])
                name = self.tryGetNameFromSceneEntities(uuid)
                if name == "unknown":
                    name = self.tryGetNameFromNameToUuidMapCache(uuid)
                if name == "unknown":
                    name = self.tryGetNameFromSavedUsers(uuid)
                    
                updateList[uuid] = name		
        self.queue.put((uiUpdateCommand, updateList))
        pass
        
    def uuidFromHex(self, hexarray):
        result = ""
        for j in range(0,hexarray.__len__()):
            hex = "%X"%ord(hexarray[j])
            if hex.__len__()==1:
                hex = "0" + hex
            result = result + hex
        
        result = result.lower()
        uuid = result[:8] + "-" + result[8:12] + "-" + result[12:16] + "-" + result[16:20] + "-" + result[20:]
        return uuid
        
            
        #u'\xc0p\x13\x90\x86~K\xb4\x9e\x7f\xb3\xf0\x1d\xb7\x94A', u'\xc4\x83\x82\x83\x07\xd0K\x93\xbb\x0f1V0\x9a\xc0\x07'
        #c0701390-867e-4bb4-9e7f-b3f01db79441                       c4838283-07d0-4b93-bb0f-3156309ac007 |
		#C0701390 867E 4BB4 9E7F B3F01DB79441						C4838283  7D0 4B93 BB F 3156309AC0 7
        
        #u'\xc0  p  \x13 \x90 \x86 ~   K         \xb4 \x9e \x7f \xb3  \xf0  \x1d \xb7  \x94 A', 
        #    c0  70   13   90  -86 7e- 4b          b4  -9e   7f  -b3    f0    1d   b7    94 41                       
        
        #u'\xc4  \x83  \x82  \x83  \x07  \xd0 K     \x93  \xbb  \x0f 1  V  0     \x9a  \xc0  \x07'
        #    c4    83    82    83   -07    d0-4b      93   -bb    0f-31 56 30      9a    c0    07 |

        #(u'7', u'8', u'0', u'0', u'0', u'2', u'\xc0p\x13\x90\x86~K\xb4\x9e\x7f\xb3\xf0\x1d\xb7\x94A', u'\xc4\x83\x82\x83\x07\xd0K\x93\xbb\x0f1V0\x9a\xc0\x07')


import sys
import time
import dbus.glib
import Connection
import traceback
import rexviewer as r
import os        
import dbusmanager
        
class IMDemo:
    def __init__(self):
        self.dbusmanager_ = dbusmanager.DBusManager()
        if not self.dbusmanager_.is_dbus_service_running():
            print("TRY to start dbus manager")
            # The paths are related to current working directory
            self.dbusmanager_.start_dbus_service("dbus/dbus-daemon.exe", "--config-file=dbus/data/session.conf")
            
        if not self.dbusmanager_.is_dbus_service_running():
            print("ERROR: Cannot find dbus service!")        
            # todo: report this error to CommunicationManager
            return
        else:
            print(">> dbus daemon running")
            
        self.connection = Connection.Connection(self)
        pass
    

##    EVENTS
    def disconnected(self, reason):
        r.pyEventCallback("disconnected","")
        
    def connecting(self, reason):
        print "CONNECTING"
        #self.ui.status = 1
        r.pyEventCallback("connecting","")

    def connected(self, reason):
        print "CONNECTED"
        #self.ui.status = 2
        r.pyEventCallback("connected","")
        self.connection.SubscribeContactList()

        
##==========================================================================================
##          new events
##==========================================================================================
    def channelOpened(self, addr):
        r.pyEventCallback("channel_opened",addr)
    def sendMess(self, reason):
        r.pyEventCallback("message_sent","")
    def receivedMess(self, addr_mess):
        r.pyEventCallback("message_received",addr_mess)
    def channelClosed(self, reason):
        r.pyEventCallback("channel_closed","")
    def gotContactlistItem(self, id_addr):
        print "gotContactlistItem"
        r.pyEventCallback("contact_item",id_addr)
    def contactStatusChanged(self, id_status):
        print "id_status: ", id_status
        r.pyEventCallback("contact_status_changed", id_status)
    # contact events
    def contactAddedToPublishList(self, addr):
        r.pyEventCallback("contact_added_publish_list", addr)
    def contactAdded(self, id):
        print "contactAdded"
        r.pyEventCallback("contact_added", id)
    def contactRemoved(self, id):
        r.pyEventCallback("contact_removed", id)
    def remotePending(self, id):
        print "remote pending event"
        pass
        #r.pyEventCallback("remote_pending", id)
    def localPending(self, id):
        r.pyEventCallback("local_pending", id)
    def incomingRequest(self, addr):
        r.pyEventCallback("incoming_request", addr)
        pass
    def gotAvailableStatuses(self, slist_N):
        r.pyEventCallback("got_available_status_list", slist_N)

    def registeringAccountSucceeded(self, reason):
        r.pyEventCallback("account_registering_succeeded", str(reason))
        pass
    def registeringAccountFailed(self, reason):
        r.pyEventCallback("account_registering_failed", str(reason))
        pass
    

##==========================================================================================
##          new api
##==========================================================================================

    def CDoStartUp(self):
        print "*********************************************"
        print "*****     IM STARTUP  SCRIPT           ******"
        print "*********************************************"
        
    def CAccountConnect(self):
        print "CAccountConnect"
        #str, d = doReadAccountAndConnect()
        d = self.connection.GetConnectionSettings()
        #self.accountConnect(d)
        self.connection.ConnectAccount(d)

    def CDisconnect(self):
        self.connection.Disconnect()

    def CStartChatSession(self, jid):
        print "Got:"
        print jid
        #self.chatEndPoint = jid
        self.connection.StartChat(jid)

    def CCloseChannel(self, addr):
        print "CCloseChannel:"
        print "calling connection.close_channel"
        self.connection.close_channel(addr)

    def CSendChat(self, addr_mess):
        spl = addr_mess.split(':')
        addr = spl[0]
        spl2 = spl[1:]
        sep = ':'
        mess = sep.join(spl2)
        print addr
        print mess
        self.connection.SendMessage(addr, mess)

    def CGetContactWithID(self, id):
        print "CGetFriendWithID"
        print id
        contact = str(self.connection.get_contact_with_id(id))
        print "Contact"
        print str(contact)
        return contact        

    def CGetStatusWithID(self, id):
        return str(self.connection.get_contact_status(id))
    
    def CSetStatus(self, status_message):
        spl = status_message.split(':')
        if len(spl)==1:
            self.connection.SetStatus(status_message, "")
        else:
            try:
                self.connection.SetStatus(spl[0], spl[1])
            except:
                print "status failure"
                tb = traceback.format_exception(*sys.exc_info())
                print ''.join(tb)                

    def CAddContact(self, contact):
##        print "in CAddContact"
        self.connection.add_contact(contact)

    def CRemoveContact(self, contact_str):
        print "CRemoveContact"
        print contact_str
        self.connection.remove_contact(contact_str)

    def CAcceptContactRequest(self, addr): #addr or id?
        print "CAcceptContactRequest"
        print addr
        self.connection.acceptLocalPending(addr)

    def CDenyContactRequest(self, addr):
        print "CDenyContactRequest"
        print addr
        self.connection.denyLocalPending(addr)

    def CRefreshContactStatusList(self):
        print "CRefreshContactStatusList"
        self.connection.UpdatePresences()

    def CSendSubscription(self, addr):
        print "CSendSubscription"
        self.connection.Subscribe(addr)
    def CGetSettings(self):
        print "CGetSettings"
        settings = self.connection.GetUISettings()
        return settings
    def CSaveSettings(self, attr_N):
        print "CSaveSettings"
        settings = self.connection.SaveSettings(attr_N)
    def CCreateAccount(self):
        print "CCreateAccount"
        ret = self.connection.CreateAccount()
        
    def CTest(self):
        self.connection.test()
    

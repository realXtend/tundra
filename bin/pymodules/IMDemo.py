##
## Will hopefully be refactored soon
##


import sys
import time
#import gobject
import dbus.glib
import Connection
import traceback
import rexviewer as r
        
        

##====================================================
##    MAIN APPLICATION LOGIC
##====================================================
class IMDemo:
    def __init__(self):
        self.chatEndPoint = None        
        self.quit = False
        self.loop = None
        self.connection = Connection.Connection(self)
        pass
    
    def setConnection(self, _conn):
        self.connection = _conn

##    EVENTS
    def disconnected(self, reason):
        #self.ui.status = 0
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
    def channelOpened(self, reason):
        r.pyEventCallback("channel_opened","")
    def sendMess(self, reason):
        r.pyEventCallback("message_sent","")
    def receivedMess(self, reason):
        r.pyEventCallback("message_received",reason)
    def channelClosed(self, reason):
        r.pyEventCallback("channel_closed","")
    def gotFriendlistItem(self, reason):
        r.pyEventCallback("contact_item",reason)
    def contactStatusChanged(self, id_status):
        r.pyEventCallback("contact_status_changed", id_status)

    # contact events
    def contactAddedToPublishList(self, addr):
        r.pyEventCallback("contact_added_publish_list", addr)
    def contactAdded(self, id):
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
    
##==========================================================================================
        
        


##==========================================================================================
##          new api
##==========================================================================================

    def CDoStartUp(self):
        print "*********************************************"
        print "*****     IM STARTUP  SCRIPT           ******"
        print "*********************************************"
        connection = Connection.Connection(self)
        self.setConnection(connection)
        
    def CAccountConnect(self):
        #str, d = self.ui.doReadAccountAndConnect()
        str, d = doReadAccountAndConnect()
        #self.accountConnect(d)
        self.connection.ConnectAccount(d)

    def CDisconnect(self):
        self.connection.Disconnect()

    def CStartChatSession(self, jid):
        print "Got:"
        print jid
        self.chatEndPoint = jid
        self.connection.StartChat(jid)

    def CCloseChannel(self):
        print "CCloseChannel:"
        print "calling connection.close_channel"
        self.connection.close_channel()

    def CSendChat(self, txt):
        self.connection.SendMessage(txt)

    def CGetFriendWithID(self, id):
        print "CGetFriendWithID"
        print id
        print type(id)
        contact = str(self.connection.get_contact_with_id(id))
        print "Contact"
        print type(contact)
        print str(contact)
        return contact        

    def CGetStatusWithID(self, id):
        return str(self.connection.get_contact_status(id))

    def CAddContact(self, contact_str):
        print "=============================================="
        print "in CAddContact"
        print "=============================================="
        self.connection.add_contact(contact_str)

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

    def CTest(self):
        self.connection.test()
    
    

        
##==========================================================================================


##gobject.type_register(IMDemo)
##gobject.signal_new("disconnect_signal", IMDemo, gobject.SIGNAL_RUN_FIRST, gobject.TYPE_NONE, ())
##gobject.signal_new("open_channel_signal", IMDemo, gobject.SIGNAL_RUN_FIRST, gobject.TYPE_NONE, ())

        


def doReadAccountAndConnect():
    d = loadAccountFile()
    return "accountconnect", d

def loadAccountFile():
    print "loading account.txt file"
    #accoutFileStr = file("Account.txt").read()
    accoutFileStr = file("pymodules/Account.txt").read()
    d = parse_account(accoutFileStr)
    return d

# copied from telep example
def parse_account(s):
    lines = s.splitlines()
    pairs = []
    
    manager = None
    protocol = None

    for line in lines:
        if not line.strip():
            continue

        k, v = line.split(':', 1)
        k = k.strip()
        v = v.strip()

        if k == 'manager':
            manager = v
        elif k == 'protocol':
            protocol = v
        else:
            if k not in ("account", "password"):
                if v.lower() == "false":
                    v = False
                elif v.lower() == "true":
                    v = True
                else:
                    try:
                        v = dbus.UInt32(int(v))
                    except:
                        pass
            pairs.append((k, v))

    d = dict(pairs)
    

    a = d['account']
    print a
    return d
        
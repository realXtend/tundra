##
## Will hopefully be refactored soon
##

import sys
import dbus.glib
import logging
import telepathy

import IMDemo
import ContactList
import IMSession

import time
from telepathy.client.channel import Channel
from telepathy.interfaces import CONN_MGR_INTERFACE, CONN_INTERFACE_PRESENCE, CONNECTION_INTERFACE_SIMPLE_PRESENCE
from telepathy.interfaces import CHANNEL_TYPE_TEXT, CONN_INTERFACE, CHANNEL_INTERFACE_GROUP, CHANNEL_TYPE_CONTACT_LIST
from telepathy.constants import (
    CONNECTION_HANDLE_TYPE_CONTACT, CONNECTION_STATUS_CONNECTED,
    CHANNEL_TEXT_MESSAGE_TYPE_NORMAL, CONNECTION_STATUS_DISCONNECTED, CONNECTION_HANDLE_TYPE_LIST)


from threading import Thread
from Queue import Queue
import traceback

class Connection():
    
    def __init__(self, app):
        self.manager_file = "gabble.manager"
        self.manager = "gabble"
        self.protocol = "jabber"
        self.conn = None
        self.cb_app = app
        self.contactlist = ContactList.ContactList(None, app)

        self.chat_address = None
        self.textchannel = None
        self.contactlistInit = False
##        self.match_sent = None
##        self.match_received = None
##        self.match_senderror = None
##        self.match_closed = None
        self.current_precenses = None        
        self.precenseCallBackSet = False

        #contact list subscribed status map
        self.presence = {}
        self.requested_contacts = {}
        self.requested_contacts_phase_map = {} # need to be aware of number of added messages

        self.sessions = {} # map of IM sessions (address & IMSession class)

        
    def _getRequestedContact(self, h):
        if (self.requested_contacts.__contains__(h)==True):
            return self.requested_contacts[h]
        else:
            return "None contact"

        
##====================================================
##    CONNECTING
##====================================================
    def ConnectAccount(self, d):
        self.account = d["account"]
        self.password = d["password"]
        self.server = d["server"]
        self.port = d["port"]
        self.old_ssl = d["old-ssl"]
        self.ignore_ssl_errors = d["ignore-ssl-errors"]
        reg = telepathy.client.ManagerRegistry()
        reg.LoadManager(self.manager_file)
        mgr = reg.GetManager(self.manager)
        
        p = d["port"]
        p0 = dbus.UInt32(int(p))
        d["port"] = p0

        conn_bus_name, conn_object_path = mgr[CONN_MGR_INTERFACE].RequestConnection(self.protocol, d)
        
        self.conn = telepathy.client.Connection(conn_bus_name, conn_object_path, ready_handler=None)
        self.conn[CONN_INTERFACE].connect_to_signal('StatusChanged', self.status_changed_cb)
        self.conn[CONN_INTERFACE].connect_to_signal('NewChannel', self.new_channel_cb)
        self.contactlist.SetConnection(self.conn)

        self._connect()
        pass

    def RecvDisconnect(self, sender):
        self.Disconnect()
        pass

    def Disconnect(self):
        if self.conn!=None:
            print "disconnecting"
            self._disconnect()


    def _connect(self):
        self.conn[CONN_INTERFACE].Connect()         

    def _disconnect(self):
        self.conn[CONN_INTERFACE].Disconnect()

##====================================================
##    CLOSE CHANNEL
##====================================================
    def close_channel(self, addr):
        print "try"
        try:
            print "close_channel"
            messages = self.textchannel[CHANNEL_TYPE_TEXT].ListPendingMessages(True)
            print type(messages)

            if(messages!=None):                
                for message in messages:
                    (i0, l0, i1, i2, i3, message0) = message
                    sender = i1
                    print str(message0)
                    addr = self.get_contact_with_id(sender)
                    print addr
                    self.cb_app.receivedMess(addr + ":" + str(message0))
                    
            self.textchannel['org.freedesktop.Telepathy.Channel'].Close()
            
        except:
            print "fail"
            tb = traceback.format_exception(*sys.exc_info())
            print ''.join(tb)                


##====================================================
##    CHAT
##====================================================

    def StartChat(self, jid):
        # create text channel with counterpart
        self.chat_address = jid
        handle = self.conn[CONN_INTERFACE].RequestHandles(CONNECTION_HANDLE_TYPE_CONTACT, [self.chat_address])[0]
##        print 'got handle %d for %s' % (handle, jid)

        # channel creation
        self.conn[CONN_INTERFACE].RequestChannel(CHANNEL_TYPE_TEXT, CONNECTION_HANDLE_TYPE_CONTACT, handle, True,
            reply_handler=self.cb_channel_reply,
            error_handler=self.cb_request_channel_error)
        pass

    def cb_channel_reply(self, arg):
        print "cb_channel_reply"
        print str(arg)
        pass

    def cb_request_channel_error(self, exception):
        print "Channel creation failed" + exception
        pass

    def SendMessage(self, addr, mess):
        print "sending message"
        
        self.sessions[addr].textchannel[CHANNEL_TYPE_TEXT].Send(CHANNEL_TEXT_MESSAGE_TYPE_NORMAL, mess)
        #self.textchannel[CHANNEL_TYPE_TEXT].Send(CHANNEL_TEXT_MESSAGE_TYPE_NORMAL, mess)
        pass

    def EndChat(self):
        pass
    

##====================================================
##    EVENTS
##====================================================
        
    def status_changed_cb(self, state, reason):
        print "STATUS CHANGED CB"
        
        select = {
            telepathy.constants.CONNECTION_STATUS_DISCONNECTED: self.cb_app.disconnected,
            telepathy.constants.CONNECTION_STATUS_CONNECTING: self.cb_app.connecting,
            telepathy.constants.CONNECTION_STATUS_CONNECTED: self.cb_app.connected}
        select[state](reason)

    def new_channel_cb(self, object_path, channel_type, handle_type, handle, suppress_handler):

        if channel_type == CHANNEL_TYPE_CONTACT_LIST:
            if CONN_INTERFACE_PRESENCE in self.conn.get_valid_interfaces():
                if(self.precenseCallBackSet==False):
                    print "setting precense update callbacks"
##                    self.contactlist.ConnectToStatusUpdates()
                    
                    self.conn[CONNECTION_INTERFACE_SIMPLE_PRESENCE].connect_to_signal(
                        'PresencesChanged', self.presences_changed)
                    #self.conn[CONN_INTERFACE_PRESENCE].connect_to_signal('PresenceUpdate', self.presence_update_signal_cb)

##                    self.conn[CONN_INTERFACE_PRESENCE].GetStatuses(reply_handler=self.get_statuses_reply_cb,
##                                                                   error_handler=self.error_cb)                    
#!!                    
                    if(self.current_precenses!=None):
                        #\bug using old way to retreave precences on startup, did not know how the new way works
                        self.conn[CONN_INTERFACE_PRESENCE].RequestPresence(self.current_precenses)
                        #self.conn[CONNECTION_INTERFACE_SIMPLE_PRESENCE].GetPresences(self.current_precenses)

                    self.precenseCallBackSet = True
                    
        if channel_type == CHANNEL_TYPE_TEXT:

            print "TEXT CHANNEL OPENED"
            print str(handle)
            print str(object_path)
            print str(suppress_handler)

            #print "enter quit_chat to end chat"
            self.textchannel = Channel(self.conn.service_name, object_path)

##            ifaces = self.textchannel.GetInterfaces()
##            print str(ifaces)

            addr = self.get_contact_with_id(handle)
            print addr
            self.sessions[addr] = IMSession.IMSession(self.cb_app, self, Channel(self.conn.service_name, object_path), addr)
            self.sessions[addr].Initialize()


    def CloseMe(self, contactAddr):
        del self.sessions[contactAddr]

    def SubscribeContactList(self):
        print 'set subscribe callback'
        print "====================="
        try:
            chan = self._request_list_channel('subscribe')
            print 'subscribe: members'
                        
            self.print_members(self.conn, chan)
            #self.contactlist.SetSubscribedMembers(self.conn, chan)
            
            chan[CHANNEL_INTERFACE_GROUP].connect_to_signal('MembersChanged', self.members_changed_subscribe_cb)
        except dbus.DBusException:
            print "subscribe channel is not available *********"
        
        print 'set publish callback'
        print "====================="
        try:
            chan2 = self._request_list_channel('publish')
            print 'publish: members'
            #self.print_members(self.conn, chan2)
            chan2[CHANNEL_INTERFACE_GROUP].connect_to_signal('MembersChanged', self.members_changed_publish_cb)
        except dbus.DBusException:
            print "publish channel is not available ***********"

        print 'set stored callback'
        print "====================="
        try:
            chan3 = self._request_list_channel('stored')
            print 'stored: members'
            #self.print_members(self.conn, chan3)
            chan3[CHANNEL_INTERFACE_GROUP].connect_to_signal('MembersChanged', self.members_changed_subscribe_cb)

        except dbus.DBusException:
            print "stored channel is not available ***********"

        print 'set known callback'
        try:
            chan4 = self._request_list_channel('known')
            print 'known: members'
            #self.print_members(self.conn, chan4)
            chan4[CHANNEL_INTERFACE_GROUP].connect_to_signal('MembersChanged', self.members_changed_subscribe_cb)

        except dbus.DBusException:
            print "known channel is not available"
        
        print 'waiting for changes'
        
                


    def _request_list_channel(self, name):
        handle = self.conn[CONN_INTERFACE].RequestHandles(
            CONNECTION_HANDLE_TYPE_LIST, [name])[0]
        return self.conn.request_channel(
            CHANNEL_TYPE_CONTACT_LIST, CONNECTION_HANDLE_TYPE_LIST,
            handle, True)

    def members_changed_subscribe_cb(self, *args):
        """Listens contact list changes"""
        print 'members_changed_subscribe_cb'
##(dbus.String(u''),
##        dbus.Array([], signature=dbus.Signature('u')),
##        dbus.Array([], signature=dbus.Signature('u')),
##        dbus.Array([], signature=dbus.Signature('u')),
##        dbus.Array([dbus.UInt32(2L)], signature=dbus.Signature('u')),
##        dbus.UInt32(0L), dbus.UInt32(0L))
##<type 'tuple'>
        added = args[1]
        removed = args[2]
        local_pending = args[3]
        remote_pending = args[4]

        if added:
            print "added"
            print "contact added to local subscribe list"
            for contact in added:
                print contact
                if(self.requested_contacts_phase_map[contact]==0):
                    print "added phase 0"
                    self.requested_contacts_phase_map[contact]=1
                else:
                    print "added phase 1"
                    addr = self.conn[CONN_INTERFACE].InspectHandles(CONNECTION_HANDLE_TYPE_CONTACT, [contact])[0]
                    print addr
                    #print self._getRequestedContact(str(contact))
                    #self.cb_app.contactAdded(self._getRequestedContact(str(contact)))
                    print "!!1"
                    self.cb_app.contactAdded(addr)

                    self.RequestSubscriptionIfNotAllreadyRequested(contact)
                    del self.requested_contacts[str(contact)]
                    self.UpdatePresences()
                    
        if removed:
            print "removed"
            print "contact removed from local subscribe list"
            print removed.__len__()
            l = removed.__len__()
            
            #for i in range(0,l):
            for contact in removed:
                #contact = removed[i]
                print contact
                self.cb_app.contactRemoved(str(contact))
                
        if local_pending:
            print "local_pending"
            print "waiting local user to accept subscription"
            for contact in local_pending:
                print contact
                print self._getRequestedContact(str(contact))
                chan, addr= self._getLocalPendingContactWithHandle(contact)
                print addr
                
                self.cb_app.localPending(self._getRequestedContact(str(contact)))
        if remote_pending:
            print "remote_pending"
            print "waiting remote user to accept local subscription"
            for contact in remote_pending:
                print contact
                print self._getRequestedContact(str(contact))
                self.cb_app.remotePending(self._getRequestedContact(str(contact)))

        print args[0] ##!!

    def members_changed_publish_cb(self, *args):
        """Listens contact list changes"""
        print 'members_changed_publish_cb'
        added = args[1]
        removed = args[2]
        local_pending = args[3]
        remote_pending = args[4]

        if added:
            print "publish added"
            print "contact added to publish list"
            for contact in added:
                print contact
                if self.requested_contacts_phase_map.__contains__(contact):
                    if(self.requested_contacts_phase_map[contact]==0):
                        print "added phase 0"
                        self.requested_contacts_phase_map[contact]=1
                    else:
                        print "added phase 1"
                        addr = self.conn[CONN_INTERFACE].InspectHandles(CONNECTION_HANDLE_TYPE_CONTACT, [contact])[0]
                        print addr

                        #print self._getRequestedContact(str(contact))
                        print "!!2"
                        #self.cb_app.contactAdded(self._getRequestedContact(str(contact)))
                        self.cb_app.contactAdded(addr)

                        self.RequestSubscriptionIfNotAllreadyRequested(contact)
                        del self.requested_contacts[str(contact)]
                        self.UpdatePresences()
                else:
                    addr = self.conn[CONN_INTERFACE].InspectHandles(CONNECTION_HANDLE_TYPE_CONTACT, [contact])[0]
                    self.cb_app.contactAddedToPublishList(addr)
                    pass
                    
        if removed:
            print "publish removed"
            print "contact removed from local publish list"
            print removed.__len__()
            l = removed.__len__()
            
            for contact in removed:
                print contact
                self.cb_app.contactRemoved(str(contact))
                
        if local_pending:
            print "publish local_pending"
            print "waiting local user to accept publish presence to counterpart"
            for contact in local_pending:
                print contact
                addr = self.conn[CONN_INTERFACE].InspectHandles(CONNECTION_HANDLE_TYPE_CONTACT, [contact])[0]
                print addr
                self.cb_app.localPending(addr)
        if remote_pending:
            print "publish remote_pending"
            print "waiting for remote counterpart to accept remote publish"
            for contact in remote_pending:
                print contact
                print self._getRequestedContact(str(contact))
                self.cb_app.remotePending(self._getRequestedContact(str(contact)))
        




    def print_members(self, conn, chan):        
        print "print_members"
        current, local_pending, remote_pending = (
            chan[CHANNEL_INTERFACE_GROUP].GetAllMembers())

        self.current_precenses = current
        for member in current:
            addr = conn[CONN_INTERFACE].InspectHandles(
                    CONNECTION_HANDLE_TYPE_CONTACT, [member])[0]
            print ' - %s' % (addr)
            self.cb_app.gotContactlistItem(str(member) + ":" + addr)
            #self.cb_app.gotContactlistItem(str(member))

        if not current:
            print ' (none)'

        print "local_pending SubscribeContactList"
        for member in local_pending:
            print ' - %s' % (
                conn[CONN_INTERFACE].InspectHandles(
                    CONNECTION_HANDLE_TYPE_CONTACT, [member])[0])
        print "remote_pending SubscribeContactList"
        for member in remote_pending:
            print ' - %s' % (
                conn[CONN_INTERFACE].InspectHandles(
                    CONNECTION_HANDLE_TYPE_CONTACT, [member])[0])
                def get_contact_with_id(self, handle):
            print "get_contact_with_id"
            ihandle = int(handle)
            addr = self.conn[CONN_INTERFACE].InspectHandles(CONNECTION_HANDLE_TYPE_CONTACT, [ihandle])[0]
            return addr

    def presence_update_signal_cb(self, presences):
        print "Got PresenceUpdate:", presences
        for (handle, presence) in presences.iteritems():
            print str(handle)
            #print str(presence)
            #dbus.Struct((dbus.UInt32(0L), dbus.Dictionary({dbus.String(u'available'): dbus.Dictionary({}, signature=dbus.Signature('sv'))}, signature=dbus.Signature('sa{sv}'))), signature=None)
            (i0, d) = presence
            #print str(d)
            status = d.keys()[0]
            print "================================"
            print status
            self.presence[str(handle)]=status
            self.cb_app.contactStatusChanged(str(handle))
            #self.presence
            
    def get_contact_status(self, id):
        return self.presence[id]

    def get_statuses_reply_cb(self, statuses):
        print "================================"
        print 'get_statuses_reply_cb'
        #print "GetStatuses replied", statuses

    def error_cb(self, error):
        print "Exception received from asynchronous method call:"
        print error

    def add_contact(self, contact_str, incoming = False):
        #print dir(self.conn[CONN_INTERFACE])
        #print str(self.conn.get_valid_interfaces())
        contact = None
        handle = None
        channel_path = None
        channel = None
        
        try:
            if(incoming==False):
                if(contact_str==""):
                    contact = self.conn[CONN_INTERFACE].RequestHandles(CONNECTION_HANDLE_TYPE_CONTACT, ["matti10@jabber.se"])[0]
                    self.requested_contacts[str(contact)] = "matti10@jabber.se" #!!
                else:
                    contact = self.conn[CONN_INTERFACE].RequestHandles(CONNECTION_HANDLE_TYPE_CONTACT, [contact_str])[0]
                    self.requested_contacts[str(contact)] = contact_str
                
                self.requested_contacts_phase_map[contact] = 0

            print "add_contact: ", contact_str            
            handle = self.conn.RequestHandles(CONNECTION_HANDLE_TYPE_LIST, ['subscribe'])[0]
            channel_path = self.conn.RequestChannel(CHANNEL_TYPE_CONTACT_LIST, 
                                                     CONNECTION_HANDLE_TYPE_LIST, 
                                                     handle, True)
            channel = Channel(self.conn.service_name, channel_path)
            channel[CHANNEL_INTERFACE_GROUP].AddMembers([contact], 'contact request')
        except:
            tb = traceback.format_exception(*sys.exc_info())
            print ''.join(tb)

    def remove_contact(self, contact_str):
        contact = None
        handle = None
        channel_path = None
        channel = None
        contact = self.conn[CONN_INTERFACE].RequestHandles(CONNECTION_HANDLE_TYPE_CONTACT, [contact_str])[0]
        try:
            handle2 = self.conn.RequestHandles(CONNECTION_HANDLE_TYPE_LIST, ['publish'])[0]
            channel_path2 = self.conn.RequestChannel(CHANNEL_TYPE_CONTACT_LIST, 
                                                     CONNECTION_HANDLE_TYPE_LIST, 
                                                     handle2, True)
            channel2 = Channel(self.conn.service_name, channel_path2)
            channel2[CHANNEL_INTERFACE_GROUP].RemoveMembers([contact], 'remove contact')
        except:
            tb = traceback.format_exception(*sys.exc_info())
            print ''.join(tb)

        try:    
            handle = self.conn.RequestHandles(CONNECTION_HANDLE_TYPE_LIST, ['subscribe'])[0]
            channel_path = self.conn.RequestChannel(CHANNEL_TYPE_CONTACT_LIST, 
                                                     CONNECTION_HANDLE_TYPE_LIST, 
                                                     handle, True)
            channel = Channel(self.conn.service_name, channel_path)
            channel[CHANNEL_INTERFACE_GROUP].RemoveMembers([contact], 'remove contact')

            pass
        except:
            tb = traceback.format_exception(*sys.exc_info())
            print ''.join(tb)
        pass

    def test(self):
        chan = self._request_list_channel('publish')
        print 'publish: pending members'
        list = chan[CHANNEL_INTERFACE_GROUP].GetLocalPendingMembers()
        #print list
        for member in list:
            print ' - %s' % (
                self.conn[CONN_INTERFACE].InspectHandles(
                    CONNECTION_HANDLE_TYPE_CONTACT, [member])[0])
            chan[CHANNEL_INTERFACE_GROUP].AddMembers([member], 'accepted')
            
    def acceptLocalPending(self, addr):
        print "acceptLocalPending"
        requested_contacts = {} # empty the crap
        channel, handle = self._getLocalPendingMemberFromPublishChannel(addr)
        if(handle!=None):
            channel[CHANNEL_INTERFACE_GROUP].AddMembers([handle], 'accepted')
            addr = self.conn[CONN_INTERFACE].InspectHandles(
                    CONNECTION_HANDLE_TYPE_CONTACT, [handle])[0]
            #self.cb_app.gotContactlistItem(str(handle))
            self.cb_app.gotContactlistItem(str(handle) + ":" + addr)
            
        else:
            print "None"


    def denyLocalPending(self, addr):
        channel, handle = self._getLocalPendingMemberFromPublishChannel(addr)
        if(handle!=None):
            channel[CHANNEL_INTERFACE_GROUP].RemoveMembers([handle], 'removed')
        else:
            print "None"
        pass

    def _getLocalPendingMemberFromPublishChannel(self, addr):
        chan = self._request_list_channel('publish')
        list = chan[CHANNEL_INTERFACE_GROUP].GetLocalPendingMembers()
        for member in list:
            print "addr: ", addr
            print "inspect: ", self.conn[CONN_INTERFACE].InspectHandles(CONNECTION_HANDLE_TYPE_CONTACT, [member])[0]
            if(addr==self.conn[CONN_INTERFACE].InspectHandles(CONNECTION_HANDLE_TYPE_CONTACT, [member])[0]):
                return chan, member
        return chan, None

    def UpdatePresences(self):
        chan = self._request_list_channel('subscribe')
        current, local_pending, remote_pending = (
            chan[CHANNEL_INTERFACE_GROUP].GetAllMembers())
        self.contactlist.current_precenses = current
        #self.current_precenses = current
        if(self.contactlist.current_precenses!=None):
            self.conn[CONN_INTERFACE_PRESENCE].RequestPresence(self.contactlist.current_precenses)
        

    def _getLocalPendingContactWithHandle(self, handle):
        """Gets address of local pending handle of subscribe or publish list"""
        """Returns channel containing the handle and address"""
        pubchan = self._request_list_channel('publish')
        subchan = self._request_list_channel('subscribe')
        publist = pubchan[CHANNEL_INTERFACE_GROUP].GetLocalPendingMembers()
        sublist = subchan[CHANNEL_INTERFACE_GROUP].GetLocalPendingMembers()
        for member in publist:
            print "handle: ", handle
            print "inspect: ", str(member)
            if(str(handle)==str(member)):
                addr = self.conn[CONN_INTERFACE].InspectHandles(CONNECTION_HANDLE_TYPE_CONTACT, [member])[0] 
                print "found addr"
                print addr
                return pubchan, addr
        for member in sublist:
            print "handle: ", handle
            print "inspect: ", str(member)
            if(str(handle)==str(member)):
                addr = self.conn[CONN_INTERFACE].InspectHandles(CONNECTION_HANDLE_TYPE_CONTACT, [member])[0] 
                print "found addr"
                print addr
                return subchan, addr

    def RequestSubscriptionIfNotAllreadyRequested(self, contact):
        pubchan = self._request_list_channel('publish')
        memberList, localPending, remotePending = pubchan[CHANNEL_INTERFACE_GROUP].GetAllMembers()
        found = False
        for member in memberList:
            if(member==contact):
                found = True
        if(found==False): #SUBSCRIBE
            pubchan[CHANNEL_INTERFACE_GROUP].AddMembers([contact], 'contact request')

    def Subscribe(self, addr):
        subchan = self._request_list_channel('subscribe')
        print "Subscribe: ", addr
        #addr = self.conn[CONN_INTERFACE].InspectHandles(CONNECTION_HANDLE_TYPE_CONTACT, [member])[0]
        handle = self.conn[CONN_INTERFACE].RequestHandles(CONNECTION_HANDLE_TYPE_CONTACT, [addr])[0]
        subchan[CHANNEL_INTERFACE_GROUP].AddMembers([handle], 'subscription request')
        pass

    def presences_changed(self, args):
        # args format a{u(uss)}
        print " presences_changed "
        print "*******************"
        print args
        for key in args.iterkeys():
            #print key, args[key]
            someid, status, status_string = args[key]
            #print someid, status, status_string
            #print key, status
            id_n_status = str(key) + ":" + status + ":" + status_string
            print id_n_status
            self.presence[str(key)]=status
            #self.cb_app.contactStatusChanged(str(key))
            self.cb_app.contactStatusChanged(id_n_status)
        pass
    
##
## Will hopefully be refactored soon
##

import sys
import gobject
import dbus.glib
import logging
import telepathy
import IMDemo
import time
#import TextChannel
from telepathy.client.channel import Channel
from telepathy.interfaces import CONN_MGR_INTERFACE, CONN_INTERFACE_PRESENCE
from telepathy.interfaces import CHANNEL_TYPE_TEXT, CONN_INTERFACE, CHANNEL_INTERFACE_GROUP, CHANNEL_TYPE_CONTACT_LIST
from telepathy.constants import (
    CONNECTION_HANDLE_TYPE_CONTACT, CONNECTION_STATUS_CONNECTED,
    CHANNEL_TEXT_MESSAGE_TYPE_NORMAL, CONNECTION_STATUS_DISCONNECTED, CONNECTION_HANDLE_TYPE_LIST)



from threading import Thread
from Queue import Queue
import traceback




class Connection(gobject.GObject):
    def __init__(self, app):

        self.manager_file = "gabble.manager"
        self.manager = "gabble"
        self.protocol = "jabber"
 
        self.conn = None
        self.cb_app = app

        self.chat_address = None
        self.textchannel = None
        self.contactListChannel = None
        self.contactlistInit = False        
        self.__gobject_init__()

        # signal match objects for text channel        
        self.match_sent = None
        self.match_received = None
        self.match_senderror = None
        self.match_closed = None
        self.current_precenses = None

        self.precenseCallBackSet = False

        # contact handle map store dbus uints
        self.mapping = {}
        self.presence = {}


## for setting values other than default
    def SetPort(self, p):
        self.port = p;
    def SetServer(self, s):
        self.server = s
    def SetAccount(self, a):
        self.account = a


    def run(self):
        """Need this for catching events"""
##        self.loop = gobject.MainLoop() #!!
##        self.loop.run()
        pass

        
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
        gobject.io_add_watch(sys.stdin, gobject.IO_IN, self.user_input_handler)

         
        """Need this for catching events"""
##        self.loop = gobject.MainLoop() #!!
##        self.loop.run()

    def _disconnect(self):
        self.conn[CONN_INTERFACE].Disconnect()
##        if self.loop:
##            time.sleep(1)
##            self.loop.quit() #gobject.MainLoop() quit
##            self.loop = None

##====================================================
##    CLOSE CHANNEL
##====================================================
    def close_channel(self):
        print "try"
        try:
            print "close_channel"
            messages = self.textchannel[CHANNEL_TYPE_TEXT].ListPendingMessages(True)
            print type(messages)

            if(messages!=None):                
                for message in messages:
                    (i0, l0, i1, i2, i3, message0) = message
                    print str(message0)
                    self.cb_app.receivedMess(str(message0))
                    
            self.textchannel['org.freedesktop.Telepathy.Channel'].Close()
            
        except:
            print "fail"
            tb = traceback.format_exception(*sys.exc_info())
            print ''.join(tb)                


##====================================================
##    INPUT HANDLER
##====================================================

    def user_input_handler(self, fd, io_condition):
        if self.textchannel==None:
            print 'input> '
        else:
            print 'you> '
        sys.stdout.flush()
        input = fd.readline()
        if len(input) == 0:     # EOF
            loop.quit()
            return 0
        #print out_connection.Invoke(input)
        if self.textchannel==None:
            self.cb_app.processInputConnCB(input)
            self.cb_app.printUIConnCB()
            return 1
        else:
            # send to channel
            #if input!="quit_chat\r\n":
            if input.startswith("quit_chat")==False:
                self.textchannel[CHANNEL_TYPE_TEXT].Send(CHANNEL_TEXT_MESSAGE_TYPE_NORMAL, input)
                return 1
            else:
                print "quit here"
                try:
                    
                    messages = self.textchannel[CHANNEL_TYPE_TEXT].ListPendingMessages(True)
                    print type(messages)
                    if(messages!=None):
                        for message in messages:
                            print str(message)

                    self.textchannel['org.freedesktop.Telepathy.Channel'].Close()
                    #time.sleep(100)
                    self.cb_app.processInputConnCB(input)
                    
                except:
                    print "fail"
                    tb = traceback.format_exception(*sys.exc_info())
                    print ''.join(tb)                
                return 1
    


##====================================================
##    CHAT
##====================================================

    def RecvStartChat(self, sender):
        """ Receiving chat initialization from DemoApp not from network """
##        print "RecvStartChat"
        #self.chat_address = self.cb_app.chatEndPoint
        self.StartChat(self.cb_app.chatEndPoint)
        pass

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
##        print type(arg)
        pass

    def cb_request_channel_error(self, exception):
        print "Channel creation failed" + exception
        pass

    def SendMessage(self, mess):
        print "sending message"
        self.textchannel[CHANNEL_TYPE_TEXT].Send(CHANNEL_TEXT_MESSAGE_TYPE_NORMAL, mess)
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
##        print "new_channel_cb"
##        print channel_type

        if channel_type == CHANNEL_TYPE_CONTACT_LIST:
            if CONN_INTERFACE_PRESENCE in self.conn.get_valid_interfaces():
                if(self.precenseCallBackSet==False):
                    print "setting precense update callbacks"
                    self.conn[CONN_INTERFACE_PRESENCE].connect_to_signal('PresenceUpdate', self.presence_update_signal_cb)

                    self.conn[CONN_INTERFACE_PRESENCE].GetStatuses(reply_handler=self.get_statuses_reply_cb,
                                                                   error_handler=self.error_cb)

                    if(self.current_precenses!=None):
                        self.conn[CONN_INTERFACE_PRESENCE].RequestPresence(self.current_precenses)

                    self.precenseCallBackSet = True
                    self.contactListChannel = Channel(self.conn.service_name, object_path)
##                    self.contactListChannel = Channel(object_path, handle_type, handle)
                    
            

        
        if channel_type == CHANNEL_TYPE_TEXT:
            self.cb_app.channelOpened("")
            print "TEXT CHANNEL OPENED"
            print str(handle)
            print str(object_path)
            print str(handle)
            print str(suppress_handler)
            
            print "enter quit_chat to end chat"
            self.textchannel = Channel(self.conn.service_name, object_path)

            ifaces = self.textchannel.GetInterfaces()
            print str(ifaces)            
            
            self.match_sent = self.textchannel[CHANNEL_TYPE_TEXT].connect_to_signal('Sent', self.cb_sent)
            self.match_received = self.textchannel[CHANNEL_TYPE_TEXT].connect_to_signal('Received', self.cb_received)
            self.match_senderror = self.textchannel[CHANNEL_TYPE_TEXT].connect_to_signal('SendError', self.cb_send_error)
            #self.textchannel[CHANNEL_TYPE_TEXT].connect_to_signal('Closed', self.cb_closed)
            self.match_closed = self.textchannel["org.freedesktop.Telepathy.Channel"].connect_to_signal('Closed', self.cb_closed)
            self.cb_app.channelOpened("")

            if(self.match_sent==None):
                print "----------------------------"
                print "sent signal match == None!!!"
                print "----------------------------"
            
            messages = self.textchannel[CHANNEL_TYPE_TEXT].ListPendingMessages(True)

            if(messages!=None):                
                for message in messages:
                    print str(message)
                    #dbus.Struct((dbus.UInt32(0L), dbus.UInt32(1240230767L), dbus.UInt32(2L), dbus.UInt32(0L), dbus.UInt32(0L), dbus.String(u'faweef')), signature=None)
                    #                1               2                       3                   4               5               6                           7
                    #((i0, l0, i1, i2, i3, message0), signature0) = message
                    #  1   2    3   4   5    6        7
                    #self.cb_app.receivedMess(str(message))
                    print dir(message)
                    
                    #self.cb_app.receivedMess(str(message))
                    try:
                        (i0, l0, i1, i2, i3, message0) = message
                        print str(message0)
                        self.cb_app.receivedMess(str(message0))
                    except:
                        print "fail 3"
                    try:
                        i0, l0, i1, i2, i3, message0 = message
                        print str(message0)
                    except:
                        print "fail 4"
                    
                    
            #print 
        pass


    def cb_sent(self, timestamp, type, text):
        print "mess sent:" + text
        self.cb_app.sendMess(text)
        pass

    
    def cb_received(self, *args):
##        Received ( u: id,          u: timestamp,             u: sender,       u: type,         u: flags,        s: text )
##                 (dbus.UInt32(0L), dbus.UInt32(1237878778L), dbus.UInt32(3L), dbus.UInt32(0L), dbus.UInt32(0L), dbus.String(u'blob'))
##        print (args)
        id, timestamp, sender, type, flags, text = args

        other_end = self.get_contact_with_id(str(sender))
        print "The other one> " + other_end
        #print other_end                            
        print text
        idlist = [id]
        self.cb_app.receivedMess(other_end + ">" + text)
        self.textchannel[CHANNEL_TYPE_TEXT].AcknowledgePendingMessages(idlist)
        

    
    def cb_send_error(self, *args):
        print (args)        
        pass


    def cb_closed(self, *args):
        print "CHANNEL CLOSED!!!"
        print (args)
        if(self.match_sent!=None):
            self.match_sent.remove()
            self.match_sent=None
            print "match sent None"
        if(self.match_received!=None):
            self.match_received.remove()
            self.match_received=None
        if(self.match_senderror!=None):
            self.match_senderror.remove()
            self.match_senderror=None
        if(self.match_closed!=None):
            self.match_closed.remove()
            self.match_closed=None

        self.textchannel = None
        self.cb_app.channelClosed("")

    def SubscribeContactList(self):
        try:
            chan = self._request_list_channel('subscribe')
        except dbus.DBusException:
            print "'%s' channel is not available" % name
            return

        print 'subscribe: members'
        self.print_members(self.conn, chan)

        #chan[CHANNEL_INTERFACE_GROUP].connect_to_signal('MembersChanged',
            #lambda *args: self.members_changed_cb(name, *args))
        chan[CHANNEL_INTERFACE_GROUP].connect_to_signal('MembersChanged', self.members_changed_cb)
##        chan[CHANNEL_INTERFACE_GROUP].connect_to_signal('MembersChanged',
##            lambda *args: self.members_changed_cb('some contact', *args))

        
        print 'waiting for changes'
        
##    def UnsubscribeContactList(self):
##        print 'UnsubscribeContactList (removing callbacks)'
##        try:
##            chan = self._request_list_channel('subscribe')
##        except dbus.DBusException:
##            print "'%s' channel is not available" % name
##            return
##
##        chan[CHANNEL_INTERFACE_GROUP].unconnect_to_signal('MembersChanged',
##            lambda *args: self.members_changed_cb(name, *args))
##        print 'waiting for changes'
                


    def _request_list_channel(self, name):
        handle = self.conn[CONN_INTERFACE].RequestHandles(
            CONNECTION_HANDLE_TYPE_LIST, [name])[0]
        return self.conn.request_channel(
            CHANNEL_TYPE_CONTACT_LIST, CONNECTION_HANDLE_TYPE_LIST,
            handle, True)

    def members_changed_cb(self, *args):
##(dbus.String(u''),
##        dbus.Array([], signature=dbus.Signature('u')),
##        dbus.Array([], signature=dbus.Signature('u')),
##        dbus.Array([], signature=dbus.Signature('u')),
##        dbus.Array([dbus.UInt32(2L)], signature=dbus.Signature('u')),
##        dbus.UInt32(0L), dbus.UInt32(0L))
##<type 'tuple'>
        print str(args)
        print type(args)
        added = args[1]
        removed = args[2]
        local_pending = args[3]
        remote_pending = args[4]

        if added:
            print "added"
            print added
        if removed:
            print "removed"
            print removed
        if local_pending:
            print "local_pending"
            print local_pending
        if remote_pending:
            print "remote_pending"
            print remote_pending

        
##    def members_changed_cb(self, reason, added, removed, local_pending, remote_pending):
##        for member in added:
##            print "subscribe_members_changed_signal_cb, adding contact:", member
##        for member in removed:
##            print "remove_members_changed_signal_cb, removing contact:", member
            
##    def members_changed_cb(self, name, message, added, removed, local_pending,
##            remote_pending, actor, reason):
##        if added:
##            for handle in added:
##                print '%s: added: %d' % (name, added)
##        if removed:
##            for handle in removed:
##                print '%s: removed: %d' % (name, added)

    def print_members(self, conn, chan):
        current, local_pending, remote_pending = (
            chan[CHANNEL_INTERFACE_GROUP].GetAllMembers())

        self.current_precenses = current
        for member in current:
            print ' - %s' % (
                conn[CONN_INTERFACE].InspectHandles(
                    CONNECTION_HANDLE_TYPE_CONTACT, [member])[0])
            self.mapping[str(member)] = conn[CONN_INTERFACE].InspectHandles(CONNECTION_HANDLE_TYPE_CONTACT, [member])[0]
            self.cb_app.gotFriendlistItem(str(member))

        if not current:
            print ' (none)'

    def get_contact_with_id(self, handle):
        print "get_contact_with_id"
        print str(self.mapping)
        if(str(self.mapping)=='{}'):
            return ""
        contact = self.mapping[handle]
        if(contact==None):
            print "contact none"
            return "blob"
        return contact

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
        print "GetStatuses replied", statuses

    def error_cb(self, error):
        print "Exception received from asynchronous method call:"
        print error

    def add_contact(self, contact_str):
        print dir(self.conn[CONN_INTERFACE])
        #print str(self.conn.get_valid_interfaces())
        contact = None
        handle = None
        channel_path = None
        channel = None
        try:
            contact = self.conn[CONN_INTERFACE].RequestHandles(CONNECTION_HANDLE_TYPE_CONTACT, ["matti10@jabber.se"])[0]
            handle = self.conn.RequestHandles(CONNECTION_HANDLE_TYPE_LIST, ['subscribe'])[0]
            channel_path = self.conn.RequestChannel(CHANNEL_TYPE_CONTACT_LIST, 
                                                     CONNECTION_HANDLE_TYPE_LIST, 
                                                     handle, True)
            channel = Channel(self.conn.service_name, channel_path)
            channel[CHANNEL_INTERFACE_GROUP].AddMembers([contact], 'contact request')
        except:
            print "h1"
            tb = traceback.format_exception(*sys.exc_info())
            print ''.join(tb)                

    
        #print str(self.contactListChannel.get_valid_interfaces())
##        try:
##            self.contactListChannel[CHANNEL_INTERFACE_GROUP].AddMembers('matti10@jabber.se', 'contact request')
##        except:
##            print "fail1"




##        try:
##            #self.contactListChannel[CHANNEL_INTERFACE_GROUP].AddMembers(['matti10@jabber.se'], 'contact request')
##            self.contactListChannel[CHANNEL_INTERFACE_GROUP].AddMembers([contact], 'contact request')            
##        except:
##            print "fail2"
##            tb = traceback.format_exception(*sys.exc_info())
##            print ''.join(tb)                

        


##        try:
##            self.contactListChannel[CHANNEL_INTERFACE_GROUP].AddMembers([CONNECTION_HANDLE_TYPE_CONTACT],['matti10@jabber.se'], 'contact request')
##        except:
##            print "fail3"


            
##        chan = self._request_list_channel('subscribe')
##        chan.AddMembers([handle], "")        
        
##        print "add_contact"
##        print contact_str
##        contact = conn.RequestHandles(CONNECTION_HANDLE_TYPE_CONTACT, [contact_str])[0]
##        print "-"
##        h = conn.RequestHandles(CONNECTION_HANDLE_TYPE_LIST, ['subscribe'])[0]
##        print "-"
##        channelpath = conn.RequestChannel(CHANNEL_TYPE_CONTACT_LIST, 
##                                          CONNECTION_HANDLE_TYPE_LIST,
##                                          handle, True)
##        print "-"
##        channel = Channel(conn.service_name, channelpath)
##        print "-"
##        
##        channel[CHANNEL_INTERFACE_GROUP].AddMembers([contact], 'contact request')
        
##        if CHANNEL_TYPE_CONTACT_LIST in self.conn.get_valid_interfaces():
##            self.conn[CHANNEL_TYPE_CONTACT_LIST].AddMembers([contact_str])
##            pass
##        pass

    def remove_contact(self, contact_str):
        
        pass

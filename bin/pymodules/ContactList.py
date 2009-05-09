import sys
import dbus.glib
import logging
import telepathy
import IMDemo
import time
from telepathy.client.channel import Channel
from telepathy.interfaces import CONN_MGR_INTERFACE, CONN_INTERFACE_PRESENCE, CONNECTION_INTERFACE_SIMPLE_PRESENCE
from telepathy.interfaces import CHANNEL_TYPE_TEXT, CONN_INTERFACE, CHANNEL_INTERFACE_GROUP, CHANNEL_TYPE_CONTACT_LIST
from telepathy.constants import (
    CONNECTION_HANDLE_TYPE_CONTACT, CONNECTION_STATUS_CONNECTED,
    CHANNEL_TEXT_MESSAGE_TYPE_NORMAL, CONNECTION_STATUS_DISCONNECTED, CONNECTION_HANDLE_TYPE_LIST)



class ContactList:
    """ Extract all contact specific stuff to this class """
    def __init__(self, connection, cb):
        self.cb_app = cb
        self.conn = connection
        self.current_precenses = None
        self.loc_pending = None
        self.rem_pending = None

    def SetConnection(self, connection):
        self.conn = connection
        


    def ConnectToStatusUpdates(self):
        print "setting precense update callbacks"
        self.conn[CONNECTION_INTERFACE_SIMPLE_PRESENCE].connect_to_signal(
            'PresencesChanged', self._precensesChanged_cb)
        if(self.current_precenses!=None):
            #\bug using old way to retreave precences on startup, did not know how the new way works
            self.conn[CONN_INTERFACE_PRESENCE].RequestPresence(self.current_precenses)
        else:
            print "current_precenses = NONE"
        pass


    def SetSubscribedMembers(self, cconn, subs_chan):
        print "SetSubscribedMembers"
        self.current_precenses, self.loc_pending, self.rem_pending = (
            subs_chan[CHANNEL_INTERFACE_GROUP].GetAllMembers())

        #set current to cb_app
        for member in self.current_precenses:
            addr = cconn[CONN_INTERFACE].InspectHandles(
                    CONNECTION_HANDLE_TYPE_CONTACT, [member])[0]
            print ' - %s' % (addr)
            id_n_addr = str(member) + ":" + addr
            #self.cb_app.gotContactlistItem(str(member))
            self.cb_app.gotContactlistItem(id_n_addr)
                
##====================================================
##    ADD & REMOVE CONTACTS
##====================================================
    def AddContact(self, addr):
        pass
    def RemoveContact(self, addr):
    #def RemoveContact(self, id):
        pass
##====================================================
    
    def members_changed_subscribe_cb(self, *args):
        pass

    def members_changed_publish_cb(self, *args):
        pass

    def _precensesChanged_cb(self, args):
        # args format a{u(uss)}
        print " presences_changed "
        print args
        for key in args.iterkeys():
            print key, args[key]
            someid, status, status_string = args[key]
            id_n_status = str(key) + ":" + status + ":" + status_string
            self.cb_app.contactStatusChanged(id_n_status)
    
    
    
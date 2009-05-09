import sys
import dbus.glib
import logging
import telepathy

import IMDemo
import ContactList

import time
from telepathy.client.channel import Channel
from telepathy.interfaces import CONN_MGR_INTERFACE, CONN_INTERFACE_PRESENCE, CONNECTION_INTERFACE_SIMPLE_PRESENCE
from telepathy.interfaces import CHANNEL_TYPE_TEXT, CONN_INTERFACE, CHANNEL_INTERFACE_GROUP, CHANNEL_TYPE_CONTACT_LIST
from telepathy.constants import (
    CONNECTION_HANDLE_TYPE_CONTACT, CONNECTION_STATUS_CONNECTED,
    CHANNEL_TEXT_MESSAGE_TYPE_NORMAL, CONNECTION_STATUS_DISCONNECTED, CONNECTION_HANDLE_TYPE_LIST)

import traceback


class IMSession:
    
    def __init__(self, app, aConn, aChannel, aContact):
        self.cb_app = app
        self.connection = aConn
        self.contactAddr = aContact
        self.textchannel = aChannel        
        self.match_sent = None
        self.match_received = None
        self.match_senderror = None
        self.match_closed = None
        
    def Initialize(self):
        self.match_sent = self.textchannel[CHANNEL_TYPE_TEXT].connect_to_signal('Sent', self.cb_sent)
        self.match_received = self.textchannel[CHANNEL_TYPE_TEXT].connect_to_signal('Received', self.cb_received)
        self.match_senderror = self.textchannel[CHANNEL_TYPE_TEXT].connect_to_signal('SendError', self.cb_send_error)
        self.match_closed = self.textchannel["org.freedesktop.Telepathy.Channel"].connect_to_signal('Closed', self.cb_closed)
        self.cb_app.channelOpened(self.contactAddr)
        messages = self.textchannel[CHANNEL_TYPE_TEXT].ListPendingMessages(True)
        if(messages!=None):                
            for message in messages:
                print str(message)
                #self.cb_app.receivedMess(str(message))
                try:
                    (i0, l0, i1, i2, i3, message0) = message
                    print str(message0)
                    self.cb_app.receivedMess(self.contactAddr + ":" + str(message0))
                except:
                    print "fail 3"
                try:
                    i0, l0, i1, i2, i3, message0 = message
                    print str(message0)
                except:
                    print "fail 4"                    


    
    def cb_sent(self, timestamp, type, text):
        print "mess sent:" + text
        self.cb_app.sendMess(str(self.contactAddr) + ":" + text)

    def cb_received(self, *args):
        id, timestamp, sender, type, flags, text = args
        #other_end = self.get_contact_with_id(str(sender))
        print "The other one> " + self.contactAddr
        print text
        idlist = [id]
        self.cb_app.receivedMess(self.contactAddr + ":" + text)
        self.textchannel[CHANNEL_TYPE_TEXT].AcknowledgePendingMessages(idlist)

    def cb_send_error(self, *args):
        print (args)        

    def cb_closed(self, *args):
        print "CHANNEL CLOSED!!!"
        print args
        print str(args)
        
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
        self.connection.CloseMe(self.contactAddr)

        


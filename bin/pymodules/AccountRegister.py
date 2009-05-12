
import dbus.glib
import sys
import telepathy
from telepathy.interfaces import CONN_MGR_INTERFACE
import dbus
import Settings

from telepathy.constants import (
    CONNECTION_STATUS_CONNECTED, CONNECTION_STATUS_DISCONNECTED)
from telepathy.interfaces import (
    CONN_INTERFACE, CONN_INTERFACE_AVATARS)

registered = False

class AccountRegister():
    global registered
    def __init__(self, app):
        self.settings = Settings.Settings()
        self.cb_app = app
        self.manager = None
        self.protocol = None
        self.conn = None
        pass
    
    def Register(self):
        registered = False
        d = self.settings.LoadSettings()
        self.manager = d["manager"]
        self.protocol = d["protocol"]
        del d["manager"]
        del d["protocol"]
        d['register'] = True
        try:
            d['port'] = dbus.UInt32(int(d['port']))
        except:
            print 'fail to convert port to int'
        print d
        self.conn = self._connect(self.manager, self.protocol, d)
        self.conn[CONN_INTERFACE].connect_to_signal('StatusChanged', self._status_changed_cb)
        print 'connecting'
        self.conn[CONN_INTERFACE].Connect()

    def _status_changed_cb(self, state, reason):
        global registered
        if state == CONNECTION_STATUS_CONNECTED:
            print 'registered'
            registered = True
            self.conn[CONN_INTERFACE].Disconnect()
            self.cb_app.registeringAccountSucceeded(reason)
        elif state == CONNECTION_STATUS_DISCONNECTED:
            if not registered:
                print 'failed (reason %d)' % reason
                self.cb_app.registeringAccountFailed(reason)  

    def _connect(self, manager, protocol, account, ready_handler=None):
        reg = telepathy.client.ManagerRegistry()
        reg.LoadManager("gabble.manager")
        reg.LoadManagers()
        print "protocol, account"
        print protocol, account
        mgr = reg.GetManager(manager)
        conn_bus_name, conn_object_path = \
            mgr[CONN_MGR_INTERFACE].RequestConnection(protocol, account)
        return telepathy.client.Connection(conn_bus_name, conn_object_path,
            ready_handler=ready_handler)


import sys
import dbus
import dbus.service
import threading
from threading import Thread
import time
import subprocess        

'''
Simple manager for starting dbus-daemon (windbus) in Windows platform.
- Can check if dbus service is running
- Can start dbus service (windbus)
- Can stop started dbus service (windbus) <-- NOT WORKING WITH PYTHON 2.5
'''

# fixed properties for test object
bus_name = 'org.realxtend.test'                        # dbus name for test object
interface_name = 'org.realxtend.test'                  # interface for test object
object_path = "/org/realxtend/test/dbus_service_test"  # test object path    

class TestObject(dbus.service.Object):

    ''' A test dbus object with one dbus callable method '''
    
    def __init__(self, object_path):    
        dbus.service.Object.__init__(self, dbus.SessionBus(), object_path)    
        
    @dbus.service.method(dbus_interface=interface_name, in_signature='s', out_signature='s') 
    def testFunction(self, data=None):
        ''' Test function, just return str("ok") '''
        return "ok"

class ServiceThread(threading.Thread):
    ''' A thread that keep dbus running '''

    ''' @param application_path full path to dbus service executable with necessary attributes '''
    def __init__(self, application_path, arguments):
        Thread.__init__(self)
        self.application_path_ = application_path;
        self.arguments_ = arguments
        self.running_ = False
        self.process_ = None
        self.print_debug_info_ = True
    
    def run(self):
        self.running_ = True
        try:
            #self.set_dbusdir_environment_variable(os.getcwd())
            #print ">>>"+str(os.environ['dbusdir'])
            if self.print_debug_info_:
                print("BEGIN subprocess: ["+str(self.application_path_)+"] with ["+str(self.arguments_)+"]")
            subprocess.call([self.application_path_, self.arguments_]) # block until process is terminated
            if self.print_debug_info_:
                print("END subprocess")
        except WindowsError, (error):
            if self.print_debug_info_:
                print("END subprocess (Exception)")
                print str(error) 
        self.running_ = False
        
    def stop(self):
        if self.process_ == None:
            return
        try:
            if self.print_debug_info_:        
                print("Try to kill") #debuf message
            self.process_.kill() # <--- PROBLEM: THIS IS NOT IMPLEMENTED IN PYTHON 2.5 !!!
            if self.print_debug_info_:
                print("Killed");
        except:
            if self.print_debug_info_:
                print("Cannot kill...");
            
    def is_running(self):
      return self.running_    
      
    def set_dbusdir_environment_variable(self, value):
        import os
        os.environ['dbusdir'] = str(value)
        
        
class DBusManager:
    ''' Offer functionality to a) check if dbus service is running
                               b) start dbus-daemon (windbus)
                               c) stop started dbus-daemon (windbus) <--- Need python at least 2.6 
    '''
    
    def __init__(self):
        ''' Initialize member variables '''
        self.service_thread_ = None
        self.create_test_object_ = False
        self.bus_ = None
        self.test_object_ = None
        
        if self.create_test_object_:
            from dbus.mainloop.glib import DBusGMainLoop
            DBusGMainLoop(set_as_default=True)
            self.dbus_loop_ = DBusGMainLoop()
    
    def is_dbus_service_running(self):
        ''' return True if running otherwise False '''        
        
        if self.create_test_object_:
            return self.test_dbus_with_object()
        else:    
            return self.test_dbus_simple()
                
    def test_dbus_with_object(self):
        '''
            try to create a bus object. If that success then dbus service is running
            @todo: more sophisticatd test: expert dbus object and call a test function of that object
        '''
        try:
            self.init_bus()
            self.export_dbus_object()
            self.call_test_object_method()
            return True
        except:
            pass
        return False
        
    def init_bus(self):
        if self.bus_ == None:
            self.bus_ = dbus.SessionBus()
            # bus = dbus.SessionBus(mainloop = self.dbus_loop_) # with main loop
        
    def export_dbus_object():
        if self.test_object_ == None:
            name = dbus.service.BusName(bus_name, bus)
            self.test_object_ = TestObject(object_path)
        
    # NOTE: This is not working yet, DO NOT USE    
    def call_test_object_method():
        ''' Get reference to exported dbus object and call "test" methos 
            return value must be "ok". If this success return True otherwise return False
        '''
        obj = bus.get_object(bus_name, object_path)
        obj_interface = dbus.Interface(obj, dbus_interface=interface_name)
        ret = obj_interface.testFunction("test")
        if ret == "ok":
            return True
        else:
            return False        
        
    def test_dbus_simple(self):
        ''' Just test if it's possible to get bus object or not '''
        try:
            self.init_bus()
            return True
        except:
            pass
        return False    

    def start_dbus_service(self, app_path, app_args):
        ''' Start dbus-daemon from given path with given arguments
            Does nothing if dbus service is already running
            
            This starts a thread wich blocks untila dbus-daemon quits
        
            @param app_path path to dbus daemon application eg.  "../dbus-daemon.exe"
            @param app_args arguments for dbus daemon application eg. "--config-file=../data/session.conf"
        '''
        
        if self.is_dbus_service_running():
            return # service already running, no need to start antother one
            
        try:    
            self.service_thread_ = ServiceThread(app_path, app_args)
            self.service_thread_.setDaemon(True) 
            self.service_thread_.start()
        except str,(reason):
            print("Cannot start dbus daemon: " + str(reason) )
        
        # Check that service exist
        # * waits a couple of seconds so dbus service can really start up
        wait_time = 5
        wait_interval = 1
        start_time = time.time()
        while not self.is_dbus_service_running() and time.time() - start_time < wait_time:
            time.sleep(wait_interval)
            
    def stop_dbus_service(self):
        ''' Stop the dbus service only if it has been started by this manager '''
     
        if self.service_thread_ == None:
            return
        self.service_thread_.stop()
        
        
if __name__ == "__main__":
    ''' Testing module functionality: Prints dbus service status
        If service is not running then:
            try start and stop the service
        else do nothing    
    '''
    
    print "DBusManager:"
    dbus_manager = DBusManager()
    running = dbus_manager.is_dbus_service_running()
    if running:
        print("* dbus service is running")
    else:
        print("* dbus service is not running")
        dbus_manager.start_dbus_service("../dbus/dbus-daemon.exe", "--config-file=../dbus/data/session.conf")
        if dbus_manager.is_dbus_service_running():
            print("* Try to start dbus: Succeed")
            dbus_manager.stop_dbus_service()
            if not dbus_manager.is_dbus_service_running():
                print("* Try to stop dbus: Succeed")
            else:
                print("* Try to stop dbus: Fail")       
        else:
            print("* Try to start dbus: Fail")
            dbus_manager.stop_dbus_service() # debug
        

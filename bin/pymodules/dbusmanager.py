import sys
import dbus
from dbus.mainloop.glib import DBusGMainLoop
import dbus.service
import threading
from threading import Thread
import time
import subprocess        

bus_name = 'org.realxtend.test'
interface_name = 'org.realxtend.test'
object_path = "/org/realxtend/test/dbus_service_test"    

class TestDBusObjectObject(dbus.service.Object):
    ''' A test object with one dbus callable method '''
    def __init__(self, object_path):    
        dbus.service.Object.__init__(self, dbus.SessionBus(), object_path)    
    # '''    def __init__(self, object_path):
        # #dbus.service.Object.__init__(self, bus, object_path)
        # dbus.service.Object.__init__(self, bus_name, object_path)
        # print "Initialized a TestObject object: " + str(object_path)'''    
        
    @dbus.service.method(dbus_interface=interface_name, in_signature='s', out_signature='s') 
    def testFunction(self, data=None):
        ''' Test function, just return str("ok") '''
        return "ok"

class ServiceThread(threading.Thread):
    ''' command is full path to dbus service executable with necessary attributes '''
    
    def __init__(self, application_path, arguments):
        Thread.__init__(self)
        self.application_path_ = application_path;
        self.arguments_ = arguments
        self.running_ = False
        self.process_ = None
    
    def run(self):
        WAIT_DELAY = 1.0
        self.running_ = True
        print "RUN"# debug
       
#        try:
        self.process_ = subprocess.Popen([self.application_path_, self.arguments_])
        std_out, std_err = self.process_.communicate()
#            subprocess.call([self.application_path_, self.arguments_])        
        # except:
            # pass
        print "STOP" # debug
        self.running_ = False
        
    def stop(self):
        if self.process_ == None:
            return

        print("KILL") #debuf message
        self.process_.kill() # <--- PROBLEM: THIS IS NOT IMPLEMENTED IN PYTHON 2.5 !!!
        
        
class DBusManager:
    ''' Can check if dbus is running and start the service if needed '''
    
    def __init__(self):
        self.service_thread_ = None
        DBusGMainLoop(set_as_default=True)
        self.dbus_loop_ = DBusGMainLoop()
    
    def is_dbus_service_running(self):
        ''' return True if running otherwise False '''
        # TEST: * Try to export object to dbus -> success or fail
        #       * In future more clean solution would be try to also use that object by calling one test function
        try:
            # create dbus object
            bus = dbus.SessionBus(mainloop = self.dbus_loop_)
            name = dbus.service.BusName(bus_name, bus)
            object = TestDBusObjectObject(object_path)
            
            # call function of dbus object
#            obj = bus.get_object(bus_name, object_path)
#            obj_interface = dbus.Interface(obj, dbus_interface=interface_name)
#            ret = obj_interface.testFunction("test")
#            ret = False
#            if ret == "ok":
            return True
                
        except:
            # dbus service test failed: Cannot export object to dbus
            pass
            
        return False
        
    def start_dbus_service(self):
        ''' Does nothing if dbus service is already running '''
        
        if self.is_dbus_service_running():
            return # service already running, no need to start
            
        self.service_thread_ = ServiceThread("../dbus-daemon.exe", "--config-file=../data/session.conf")
        self.service_thread_.setDaemon(True) 
        self.service_thread_.start()
        
        # wait a couple of seconds so dbus service can really start up
        wait_time = 10
        wait_interval = 1
        start_time = time.time()
        while not self.is_dbus_service_running() and time.time()-start_time < wait_time:
            print "."
            time.sleep(wait_interval)
            

        
    def stop_dbus_service(self):
        ''' Stop the dbus service only if it has been started by this manager '''
     
        if self.service_thread_ == None:
            return
        self.service_thread_.stop()
        
        
if __name__ == "__main__":
    ''' Prints dbus service status and try start and stop the service if isn't available '''
    print "DBusManager:"
    dbus_manager = DBusManager()
    running = dbus_manager.is_dbus_service_running()
    if running:
        print("* dbus service is running")
    else:
        print("* dbus service is not running")
        dbus_manager.start_dbus_service()
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
        

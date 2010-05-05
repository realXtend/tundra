import time

try:
    import rexviewer as r
except ImportError: #not running under rex
    import mockviewer as r

import circuits
from circuits.core import handler

class TestThread(circuits.Thread):
    """
    A threaded circuits component inside Naali,
    now testing how this behaves, as can be needed for e.g. webdav inv
    """

    #def update(self, deltatime):
    #    if self.data == 1:
    #        self.data = 2 #change this to any non-1 number to test live reloading
    #    else:
    #        self.data = 1
    #    #r.logInfo(str(self.data))
        
    @handler("on_input") #why is this needed for Thread, but not for Component?
    def on_input(self, evid):
        print "Test Thread received input event:", evid
        #time.sleep(1)

if __name__ == '__main__':
    from circuits import Event, Manager, Debugger
    class Input(Event): pass

    m = Manager()
    c = TestThread()
    #c.start()
    m += c + Debugger()

    m.push(Input(1), "on_input")
    #c.join()

    while m:
        m.flush()


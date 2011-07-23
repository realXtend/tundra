import time
from collections import deque

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
    def __init__(self):
        circuits.Thread.__init__(self)
        self.commands = deque()
        self.start()

    #gets events from the manager in the main thread
    @handler("on_input") #why is this needed for Thread, but not for Component?
    def on_input(self, evid):
        print "Test Thread received input event:", evid

    #this is executed in a separate thread
    def run(self):
        while 1:
            time.sleep(1)
            print "I'm a lazy sleeper. And even a blocking one."
            if len(self.commands) > 0:
                print self.commands
                c = self.commands.popleft()
                if c == "stop":
                    self.stop()
                    return

    #when this worker thread stops itself
    def stop(self):
        circuits.Thread.stop(self)
        print "stopped"

    #when the manager is stopped
    @handler("stopped")
    def stopped(self, caller):
        self.commands.append("stop")

if __name__ == '__main__':
    from circuits import Event, Manager, Debugger
    class Input(Event): pass

    m = Manager()
    c = TestThread()
    m += c + Debugger()

    now = time.time()
    interval = 0.5
    prev_time = now

    try:
        while 1:
            now = time.time()
            if prev_time + interval < now:
                m.push(Input(1), "on_input")
                prev_time = now
            while m:
                m.flush()

    except KeyboardInterrupt:
        print "stopping"
        m.stop()
    
        while m:
            m.flush()
        c.join()

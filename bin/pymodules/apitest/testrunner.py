import rexviewer as r
import circuits

import time
import sys #there's no exit call in naali py api now

#user, pwd, server = "Test User", "test", "localhost:9000"
user, pwd, server = "d d", "d", "world.evocativi.com:8002"

class Init:
    def __init__(self):
        r.logInfo("TestRunner: Init")

    def do(self):
        r.startLoginOpensim(user, pwd, server)
        return Login()

class Login:
    WAITTIME = 60

    def __init__(self):
        r.logInfo("TestRunner: Login")
        self.started = time.time()

    def do(self):
        #print "_",
        if time.time() > (self.started + self.WAITTIME):
            return Logout()
        
        return self

class Logout:
    def __init__(self):
        r.logInfo("TestRunner: Logout")

    def do(self):
        r.logout()
        return Exit()

class Exit:
    WAITTIME = 30

    def __init__(self):
        self.started = time.time()

    def do(self):
        #print "_",
        if time.time() > (self.started + self.WAITTIME):
            sys.exit()
            return None
        
        return self

class TestRunner(circuits.Component):
    def __init__(self):
        circuits.Component.__init__(self)
        self.cur_state = Init() #current state. .state is reserved in Circuits. bad!

    def update(self, deltatime):
        #print "Running test in state:", self.cur_state
        if self.cur_state is not None:
            self.cur_state = self.cur_state.do()

import rexviewer as r
import circuits
import time

#user, pwd, server = "Test User", "test", "localhost:9000"
user, pwd, server = "d d", "d", "world.evocativi.com:8002"

class Init:
    def do(self):
        r.startLoginOpensim(user, pwd, server)
        return Login()

class Login:
    WAITTIME = 30

    def __init__(self):
        self.started = time.time()

    def do(self):
        #print "_",
        if time.time() > (self.started + self.WAITTIME):
            return Logout()
        
        return self

class Logout:
    def do(self):
        print "Add logout here"
        return self

class TestRunner(circuits.Component):
    def __init__(self):
        circuits.Component.__init__(self)
        self.cur_state = Init() #current state. .state is reserved in Circuits. bad!

    def update(self, deltatime):
        #print "Running test in state:", self.cur_state
        self.cur_state = self.cur_state.do()

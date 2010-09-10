import rexviewer as r
import circuits

import time

user, pwd, server = "Test Bot", "test", "world.realxtend.org:9000"
#user, pwd, server = "d d", "d", "world.evocativi.com:8002"

class TestRunner(circuits.Component):
    def __init__(self, cfsection=None):
        self.config = cfsection or dict()
        circuits.Component.__init__(self)
        self.testgen = self.run()

    def update(self, deltatime):
        prev = None
        try:
            status = self.testgen.next()
            if status:
                r.logInfo("Test state: " + str(status))
                prev = status
            #else:
                #print "Test state", prev, "still running"
        except StopIteration:
            print "Test finished"

    def run(self):
        raise NotImplementedError

    def timer_start(self):
        self.timer_started = time.time()

    def elapsed(self, n):
        return (time.time() - self.timer_started) > n
            

class TestLoginLogoutExit(TestRunner):
    def run(self):
        self.wait_time = int(self.config.get("wait_time", 30))
        yield "login"
        self.timer_start()
        r.startLoginOpensim(user, pwd, server)
        print "ELAPSED:", self.elapsed(self.wait_time)
        while not self.elapsed(self.wait_time):
            #print "X",
            yield None

        yield "logging out"
        self.timer_start()
        r.logout()
        while not self.elapsed(self.wait_time):
            yield None

        yield "exiting"
        r.exit()


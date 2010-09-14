import rexviewer as r
import circuits
import naali
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
        except StopIteration:
            # print "Test finished"
            return 

        if status:
            r.logInfo("Test state: " + str(status))
            prev = status
        #else:
            #print "Test state", prev, "still running"

    def timer_start(self):
        self.timer_started = time.time()

    def elapsed(self, n):
        return (time.time() - self.timer_started) > n

    def sceneadded(self, name):
        r.logInfo("TestRunner sceneadded called")

class TestLoginLogoutExit(TestRunner):
    def run(self):
        self.wait_time = int(self.config.get("wait_time", 60))
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

class TestCreateDestroy(TestRunner):
    def run(self):
        self.wait_time = int(self.config.get("wait_time", 300))
        yield "doing login"
        self.timer_start()
        r.startLoginOpensim(user, pwd, server)
        yield "waiting for connection"
        conn = None
        while not self.elapsed(self.wait_time):
            conn = r.getServerConnection()
            if conn and conn.IsConnected():
                break
            else:
                yield None
        else:
            return
        yield "waiting for scene"
        if 0:
            self.scene = None
            while not self.scene and not self.elapsed(self.wait_time):
                yield None
        else:
            # for some reason our sceneadded doesn't get called,
            # so fake it
            self.sceneadded("World")

        yield "creating object"
        r.getServerConnection().SendObjectAddPacket(42, 42, 22)
        self.done = False
        yield "waiting for EntityCreated"
        while not self.done and not self.elapsed(self.wait_time):
            yield None
        yield "test result: " + str(self.done)
        yield "exiting"
        r.exit()

    def sceneadded(self, name):
        #r.logInfo("CreateDestroy sceneadded called")
        self.scene = naali.getScene(name)
        self.scene.connect("EntityCreated(Scene::Entity*, AttributeChange::Type)", self.handle_entity_created)
        r.logInfo("EntityCreated callback registered")

    # qt slot
    def handle_entity_created(self, ent, changetype):
        ent_id = ent.Id
        ent = naali.getEntity(ent.Id)
        try:
            p = ent.netpos.Position
        except AttributeError:
            if 0: print "skip unplaceable entity"
        else:
            if 0: print "placeable entity created: pos", p.x(), p.y(), p.z()

            # for some reason z coord ends up as 22.25
            if p.x() == 42.0 and p.y() == 42.0 and int(p.z()) == 22:
                r.logInfo("found created test prim, deleting")
                r.getServerConnection().SendObjectDeRezPacket(
                    ent_id, r.getTrashFolderId())
                self.done = True


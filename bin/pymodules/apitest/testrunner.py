import rexviewer as r
import circuits
import naali
import time

user, pwd, server = "Test Bot", "test", "world.realxtend.org:9000"
#user, pwd, server = "d d", "d", "world.evocativi.com:8002"

class TestRunner(circuits.BaseComponent):
    def __init__(self, cfsection=None):
        self.config = cfsection or dict()
        circuits.BaseComponent.__init__(self)
        self.testgen = self.run()
    

    @circuits.handler("on_sceneadded")
    def sceneadded(self, name):
        r.logInfo("base class sceneadded callback")

    @circuits.handler("update")
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
    def __init__(self, *args, **kw):
        self.finished = False
        self.scene = None
        TestRunner.__init__(self, *args, **kw)

    def run(self):
        self.wait_time = int(self.config.get("wait_time", 60))
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
        while not self.scene and not self.elapsed(self.wait_time):
            yield None

        yield "creating object"
        r.getServerConnection().SendObjectAddPacket(42, 42, 22)
        yield "waiting for EntityCreated"
        while (not self.finished) and (not self.elapsed(self.wait_time)):
            yield None
        yield "exiting"
        r.exit()
        if self.finished:
            yield "success"
        else:
            yield "failure"

    @circuits.handler("on_sceneadded")
    def sceneadded(self, name):
        #r.logInfo("CreateDestroy sceneadded called")
        self.scene = naali.getScene(name)

        self.scene.connect("EntityCreated(Scene::Entity*, AttributeChange::Type)", self.handle_entity_created)
        r.logInfo("EntityCreated callback registered")

    # qt slot
    def handle_entity_created(self, ent, changetype):
        # fun fact: since we are called for every entity and
        # self.finished checked only every "update" event,
        # this often cleans up >1 test objects (in case any
        # are left over from failed tests)

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
                r.logInfo("found created test prim, deleting (finished=%s)" % self.finished)
                r.getServerConnection().SendObjectDeRezPacket(
                    ent_id, r.getTrashFolderId())
                self.finished = True


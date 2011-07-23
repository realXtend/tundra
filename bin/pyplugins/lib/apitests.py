
import tundra as tundra

class ApiRunner:

    def __init__(self):
        try: self.testLoggers() 
        except Exception as e: print "\n**** Python ApiRunner.testLoggers() failed ****\n", e
        try: self.testFramework() 
        except Exception as e: print "\n**** Python ApiRunner.testFramework() failed ****\n", e
        try: self.testServer() 
        except Exception as e: print "\n**** Python ApiRunner.testServer() failed ****\n", e
        try: self.testClient() 
        except Exception as e: print "\n**** Python ApiRunner.testClient() failed \n", e
        try: self.testScene() 
        except Exception as e: print "\n**** Python ApiRunner.testScene() failed ****\n", e
        try: self.testRenderer() 
        except Exception as e: print "\n**** Python ApiRunner.testRenderer() failed ****\n", e
        try: self.testHelpers() 
        except Exception as e: print "\n**** Python ApiRunner.testHelpers() failed ****\n", e
        print "\n\n"

    def testLoggers(self):
        tundra.LogInfo("\n\nPython ApiRunner starting...\n\n")
        tundra.LogInfo("-- Testing loggers")
        tundra.LogDebug("this should only show on debug mode")
        tundra.LogWarning("this is a warning")
        tundra.LogError("this is a error")
        tundra.LogFatal("this is a fatal")
    
    def testFramework(self):
        tundra.LogInfo("-- Testing framework API presence")
        print "  >> Framework()", tundra.Framework()
        print "  >> Framework().IsHeadless()", tundra.Framework().IsHeadless()
        print "  >> Frame()", tundra.Frame()
        print "  >> Scene()", tundra.Scene()
        print "  >> Asset()", tundra.Asset()
        print "  >> Audio()", tundra.Audio()
        print "  >> Input()", tundra.Input()
        print "  >> Console()", tundra.Console()
        print "  >> Ui()", tundra.Ui()
        
    def testServer(self):
        tundra.LogInfo("-- Testing Server")
        print "  >> IsServer()", tundra.IsServer()
        print "  >> Server().IsRunning()", tundra.Server().IsRunning()
        if tundra.IsServer() and not tundra.Server().IsRunning():
            print "  >> Connecting to server started signal..."
            tundra.Server().connect("ServerStarted()", self.onServerStart)
                   
    def onServerStart(self):
        tundra.LogInfo("-- Server().ServerStarted signal emitted")
        print "  >> Server().GetPort()", tundra.Server().GetPort()
        print "  >> Server().GetProtocol()", tundra.Server().GetProtocol()
        
    def testClient(self):
        tundra.LogInfo("-- Testing Client")
        print "  >> IsClient()", tundra.IsClient()
        print "  >> Client().IsConnected()", tundra.Client().IsConnected()

    def testScene(self):
        tundra.LogInfo("-- Testing Scene")
        print "  >> Scene().GetDefaultSceneRaw()", tundra.Scene().GetDefaultSceneRaw()
        if tundra.IsServer() and tundra.Scene().GetDefaultSceneRaw() == None:
            print "  >> Connecting to SceneAPI DefaultWorldSceneChanged signal..."
            tundra.Scene().connect("DefaultWorldSceneChanged(Scene*)", self.OnDefaultSceneChanged)
        if tundra.Scene().GetDefaultSceneRaw() != None:
            print "  >> Helper().CreateEntity([], True, False, False) local", tundra.Helper().CreateEntity([], True, False, False)
            print "  >> Helper().CreateEntity([], True, False, True) local temporary", tundra.Helper().CreateEntity([], True, False, True)
            print "  >> Helper().CreateEntity() replicated", tundra.Helper().CreateEntity()
            ent = tundra.Helper().CreateEntity(["EC_Mesh", "EC_Placeable"])
            print "  >> ent = Helper().CreateEntity([EC_Mesh, EC_Placeable]) replicated"
            print "  >> ent", ent
            print "  >> ent.GetComponentRaw(\"EC_Mesh\")", ent.GetComponentRaw("EC_Mesh")
            print "  >> ent.GetComponentRaw(\"EC_Placeable\")", ent.GetComponentRaw("EC_Placeable")
    
    def OnDefaultSceneChanged(self, scene):
        tundra.LogInfo("-- Scene().DefaultWorldSceneChanged signal emitted")
        print "  >> New default scene is", scene
        print "  >> Running testScene() tests againg..."
        self.testScene()
        
    def testRenderer(self):
        tundra.LogInfo("-- Testing Renderer")
        print "  >> Renderer()", tundra.Renderer()
        print "  >> Renderer().GetWindowWidth()", tundra.Renderer().GetWindowWidth()
        print "  >> Renderer().GetWindowHeight()", tundra.Renderer().GetWindowHeight()
        print "  >> rayResult = Renderer().Raycast(100,100)"
        rayResult = tundra.Renderer().Raycast(100,100)
        print "  >> rayResult", rayResult
        if rayResult != None:
            print "  >> rayResult.entity", rayResult.entity
            print "  >> rayResult.submesh", rayResult.submesh
            print "  >> rayResult.pos", rayResult.pos
        
    def testHelpers(self):
        tundra.LogInfo("-- Testing Python Helper")
        print "  >> Helper().CreateInputContext()", tundra.Helper().CreateInputContext("apirunner-context", 90)

if __name__ == "__main__":
    r = ApiRunner()

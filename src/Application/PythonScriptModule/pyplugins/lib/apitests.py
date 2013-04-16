
import tundra as tundra

class ApiRunner:

    def __init__(self):
        tundra.LogInfo("***** Python ApiRunner starting *****")
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
        tundra.LogInfo("***** Python ApiRunner done *********")
        
    def testLoggers(self):
        tundra.LogInfo("-- Testing log channels")
        tundra.LogDebug("this should only show on debug mode")
        tundra.LogWarning("this is a warning")
        tundra.LogError("this is a error")
    
    def testFramework(self):
        tundra.LogInfo("-- Testing framework API presence")
        tundra.LogInfo("  >> Framework() " + str(tundra.Framework()))
        tundra.LogInfo("  >> Framework().IsHeadless() " + str(tundra.Framework().IsHeadless()))
        tundra.LogInfo("  >> Frame() " + str(tundra.Frame()))
        tundra.LogInfo("  >> Scene() " + str(tundra.Scene()))
        tundra.LogInfo("  >> Asset() " + str(tundra.Asset()))
        tundra.LogInfo("  >> Audio() " + str(tundra.Audio()))
        tundra.LogInfo("  >> Input() " + str(tundra.Input()))
        tundra.LogInfo("  >> Console() " + str(tundra.Console()))
        tundra.LogInfo("  >> Ui() " + str(tundra.Ui()))
        
    def testServer(self):
        tundra.LogInfo("-- Testing Server")
        tundra.LogInfo("  >> IsServer() " + str(tundra.IsServer()))
        tundra.LogInfo("  >> Server().IsRunning() " + str(tundra.Server().IsRunning()))
        if tundra.IsServer() and not tundra.Server().IsRunning():
            tundra.LogInfo("  >> Connecting to server started signal...")
            tundra.Server().connect("ServerStarted()", self.onServerStart)
                   
    def onServerStart(self):
        tundra.LogInfo("-- Server().ServerStarted signal emitted")
        tundra.LogInfo("  >> Server().GetPort() " + str(tundra.Server().GetPort()))
        tundra.LogInfo("  >> Server().GetProtocol() " + str(tundra.Server().GetProtocol()))
        
    def testClient(self):
        tundra.LogInfo("-- Testing Client")
        tundra.LogInfo("  >> IsClient() " + str(tundra.IsClient()))
        tundra.LogInfo("  >> Client().IsConnected() " + str(tundra.Client().IsConnected()))
        tundra.Client().connect("Connected()", self.onClientConnected)
        
    def onClientConnected(self):
        tundra.LogInfor("-- Client::Connected() emitted")

    def testScene(self):
        tundra.LogInfo("-- Testing Scene")
        tundra.LogInfo("  >> Scene().GetDefaultSceneRaw() " + str(tundra.Scene().GetDefaultSceneRaw()))
        if tundra.IsClient() and tundra.Scene().GetDefaultSceneRaw() == None:
            tundra.LogInfo("  >> Scene not yet there and this is a client run, nothing to test for scene.")
        if tundra.IsServer() and tundra.Scene().GetDefaultSceneRaw() == None:
            tundra.LogInfo("  >> Connecting to SceneAPI DefaultWorldSceneChanged signal...")
            tundra.Scene().connect("DefaultWorldSceneChanged(Scene*)", self.OnDefaultSceneChanged)
        if tundra.Scene().GetDefaultSceneRaw() != None:
            tundra.LogInfo("  >> Helper().CreateEntity([] + True + False + False) local " + str(tundra.Helper().CreateEntity([], True, False, False)))
            tundra.LogInfo("  >> Helper().CreateEntity([] + True + False + True) local temporary " + str(tundra.Helper().CreateEntity([], True, False, True)))
            tundra.LogInfo("  >> Helper().CreateEntity() replicated" + str(tundra.Helper().CreateEntity()))
            ent = tundra.Helper().CreateEntity(["EC_Mesh", "EC_Placeable"])
            tundra.LogInfo("  >> ent = Helper().CreateEntity([EC_Mesh + EC_Placeable]) replicated")
            tundra.LogInfo("  >> ent " + str(ent))
            tundra.LogInfo("  >> ent.GetComponentRaw(\"EC_Mesh\") " + str(ent.GetComponentRaw("EC_Mesh")))
            tundra.LogInfo("  >> ent.GetComponentRaw(\"EC_Placeable\") " + str(ent.GetComponentRaw("EC_Placeable")))
    
    def OnDefaultSceneChanged(self, scene):
        tundra.LogInfo("-- Scene().DefaultWorldSceneChanged signal emitted")
        tundra.LogInfo("  >> New default scene is " + str(scene))
        tundra.LogInfo("  >> Running testScene() tests againg...")
        self.testScene()
        
    def testRenderer(self):
        tundra.LogInfo("-- Testing Renderer")
        tundra.LogInfo("  >> Renderer() " + str(tundra.Renderer()))
        tundra.LogInfo("  >> Renderer().GetWindowWidth() " + str(tundra.Renderer().GetWindowWidth()))
        tundra.LogInfo("  >> Renderer().GetWindowHeight() " + str(tundra.Renderer().GetWindowHeight()))
        tundra.LogInfo("  >> rayResult = Renderer().Raycast(100,100)")
        rayResult = tundra.Renderer().Raycast(100, 100)
        tundra.LogInfo("  >> rayResult " + str(rayResult))
        if rayResult != None:
            tundra.LogInfo("  >> rayResult.entity " + str(rayResult.entity))
            tundra.LogInfo("  >> rayResult.submesh " + str(rayResult.submesh))
            tundra.LogInfo("  >> rayResult.pos " + str(rayResult.pos))
        
    def testHelpers(self):
        tundra.LogInfo("-- Testing Python Helper")
        tundra.LogInfo("  >> Helper().CreateInputContext() " + str(tundra.Helper().CreateInputContext("apirunner-context", 90)))

if __name__ == "__main__":
    r = ApiRunner()

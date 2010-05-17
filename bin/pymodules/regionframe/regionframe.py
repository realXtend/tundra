import rexviewer as r
from circuits import Component

try:
    window
except:
    try:
        import window
    except ImportError, e:
        print "couldn't load window: ", e
else:
    window = reload(window)

class RegionFrame(Component):

    def __init__(self):
        Component.__init__(self)
        self.window = window.RegionFrameWindow(self)

    def on_genericmessage(self, name, data):
        if name == "regionframelist":
            if type(data) == type(list()):
                self.window.PopulateList(data)
            #print "RegionFrame Generic message handler got data: ", data

    def listFrames(self):
        worldstream = r.getServerConnection()
        worldstream.SendGenericMessage("regionframe", ["list"])
        
    def clearFrames(self):
        worldstream = r.getServerConnection()
        worldstream.SendGenericMessage("regionframe", ["clear"])
        
    def saveFrame(self):
        frameName = self.window.GetSaveFileName()
        worldstream = r.getServerConnection()
        worldstream.SendGenericMessage("regionframe", ["save", frameName])

    def unloadFrame(self):
        frameName = self.window.GetSaveFileName()
        worldstream = r.getServerConnection()
        worldstream.SendGenericMessage("regionframe", ["remove", frameName])
        
    def loadFrame(self):
        frameName = self.window.GetSaveFileName()
        worldstream = r.getServerConnection()
        worldstream.SendGenericMessage("regionframe", ["load", frameName])

    def on_exit(self):
        r.logInfo("Region Frame exiting...")

        self.window.on_exit()

        r.logInfo("         ...exit done.")

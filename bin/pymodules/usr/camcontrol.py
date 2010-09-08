from circuits import Component
import rexviewer as r
import naali

inputmap = {
    r.MoveForwardPressed: 0.1
#    r.MoveBackwardPressed: -0.1
}

class CameraController(Component):
    def __init__(self):
        Component.__init__(self)
        self.scene = None #now just to know when can perhaps soon get the cam
        self.cam = None

    def on_sceneadded(self, name):
        #camera is only created upon login when the scene is created .. or a bit after :p
        self.scene = naali.getScene(name)

    def update(self, t):
        if self.cam is None and self.scene is not None:
            cament = naali.getCamera()
            self.cam = cament
            r.switchCameraState() #supposes that FREECAM was off, turns it on.
            #old quick hack reused for now, good idea is probably to create an own cam ent for this
            
    def on_input(self, evid):
        if self.cam is not None and evid in inputmap:
            #oldpos = self.cam.Placeable.Position
            self.cam.placeable.translate(0, inputmap[evid])
            print ".",
            return True #event was handled, i.e. overrides this key
        else:
            print self.cam, evid, inputmap
        
    def on_logout(self, id):
        self.cam = None
        self.scene = None


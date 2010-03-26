import dotscene
try:
    import rexviewer
except ImportError:
    print "NOTE: dotscene loader not able to access Naali API, is running outiside Naali?"
    rexviewer = None
else:
    import PythonQt.QtGui
    Vec = PythonQt.QtGui.QVector3D
    Quat = PythonQt.QtGui.QQuaternion

class OgreNode:
    """the equivalent of Ogre Node in Naali for this import now.
    perhaps this will wrap or just be the Naali .. placeable E-C?"""

    def __init__(self):
        #these are set by the parser directly
        self.position = None
        self.orientation = None
        self.scale = None
        self.object = None
        self.naali_ent = None

    def createChildSceneNode(self):
        return OgreNode()

    def attachObject(self, ob):
        self.object = ob
        print self, "attached object", ob
        if rexviewer is not None: 
            self.create_naali_meshentity()

    def create_naali_meshentity(self):
        self.naali_ent = rexviewer.createEntity(self.object.mesh, 100002) #XXX handle name too. what about id?
        e = self.naali_ent
        print "Created naali entity:", e, e.id
            
        """apply pos, rot & scale. 
        XXX this could skip all conversions, like quat now, 
        by dotscene.py doing the qt types directly"""
        p = e.placeable
        #print p.Position.toString(), self.position
        p.Position = Vec(*self.position)
            
        #print p.Orientation.toString(), self.orientation
        p.Orientation = self.orientation

        p.Scale = Vec(*self.scale)

class OgreEntity:
    def __init__(self, name, mesh):
        self.name = name
        self.mesh = mesh

class NaaliSceneManagerFacade:
    """
    implements the ogre SceneManager interface
    enough for the dotscene parser to work.

    kudos to mikkopa on the modrex side for coming up
    with this alternative SM impl trick in the c# loader there.
    """

    def __init__(self):
        self.rootnode = OgreNode()

    def getRootSceneNode(self):
        return self.rootnode

    def createEntity(self, name, mesh):
        print "Creating entity", name, mesh
        return OgreEntity(name, mesh)

    #def createLight(self, name):
    #    pass

    #def createCamera(

def load_dotscene(fname):
    sm = NaaliSceneManagerFacade()
    ds = dotscene.DotScene(fname, sm)

if __name__ == '__main__':
    load_dotscene("test.scene")

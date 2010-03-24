import dotscene

class OgreNode:
    """the equivalent of Ogre Node in Naali for this import now.
    perhaps this will wrap or just be the Naali .. placeable E-C?"""

    def __init__(self):
        #these are set by the parser directly
        self.position = None
        self.orientation = None
        self.scale = None
        self.ob = None

    def createChildSceneNode(self):
        return OgreNode()

    def attachObject(self, ob):
        self.ob = ob
        print self, "attached object", ob

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
    

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
    
        self.id = None
        self.entityNode = None
        self.entityName = None
        self.entityMeshFile = None
        self.entityStatic = None
        
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
        #p.Position = Vec(*self.position)
        mp = self.position
        #p.Position = Vec(-mp[0], mp[2], mp[1])
        #p.Position = Vec(mp[0]+127, mp[1]+127, mp[2]+200)
        p.Position = Vec(mp[0]+127, mp[1]+127, mp[2]+25)
        #print p.Position.toString(), self.position
            
        #print p.Orientation.toString(), self.orientation
        mo = self.orientation #mock to where dotscene was read
        o = Quat(mo.w, mo.x, mo.y, mo.z)
        #print dir(o)
        o.__imul__(Quat(1, 1, 0, 0))
        #o.__imul__(Quat(1, 0, 1, 0))
        p.Orientation = o
        #print p.Orientation.toString(), o.toString()

        p.Scale = Vec(*self.scale)

"""
the server side importer in modrex does conversions like this:
                        Quaternion rot = new Quaternion(node.DerivedOrientation.X, node.DerivedOrientation.Y, node.DerivedOrientation.Z, node.DerivedOrientation.W);
                        if (m_swapAxes == true)
                        {
                            Vector3 temp = new Vector3(rot.X, rot.Y, rot.Z);
                            rot.X = -temp.X;
                            rot.Y = temp.Z;
                            rot.Z = temp.Y;
                        }
                        else
                        {
                            // Do the rotation adjust as in original importer
                            rot *= new Quaternion(1, 0, 0);
                            rot *= new Quaternion(0, 1, 0);
                        }
                        rot = sceneRotQuat * rot;
                        
                        SceneObjectGroup sceneObject = m_scene.AddNewPrim(m_scene.RegionInfo.EstateSettings.EstateOwner,
                            m_scene.RegionInfo.EstateSettings.EstateOwner, objPos, rot, PrimitiveBaseShape.CreateBox());
                        Vector3 newScale = new Vector3();
                        newScale.X = node.DerivedScale.X * m_objectScale;
                        newScale.Y = node.DerivedScale.Y * m_objectScale;
                        newScale.Z = node.DerivedScale.Z * m_objectScale;
                        if (m_swapAxes == true)
                        {
                            Vector3 temp = new Vector3(newScale);
                            newScale.X = temp.X;
                            newScale.Y = temp.Z;
                            newScale.Z = temp.Y;
                        } 
                        sceneObject.RootPart.Scale = newScale;
"""

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
    return ds, ds.dotscenemanager

def unload_dotscene(ds):
    print "unload_dotscene"
    for k, oNode in ds.dotscenemanager.nodes.iteritems():
        print "removing " + k
        print oNode.naali_ent.id
        try:
            rexviewer.removeEntity(oNode.naali_ent.id)
        except:
            print "failed in unload_dotscene"

    
if __name__ == '__main__':
    load_dotscene("test.scene")

    
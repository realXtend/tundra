import circuits
import naali

COMPNAME = "materialtemplate"

class MaterialTemplateHandler(circuits.BaseComponent):
    def __init__(self, entity=None, comp=None, changetype=None):
        circuits.BaseComponent.__init__(self)
        self.entity = entity
        
        comp.connect("library.TextureUrlWasAssigned(uint submesh)", self.newTexture)
        
    def newTexture(self, submesh):
        print "Submesh ", submesh, " got a new texture img"
        print "Testing..."
        print self.entity.prim.Materials[submesh]
        
        
        # 
        # % {"name": self.getNewName() , "texture": str(self.entity.prim.Materials[submesh]) }
        # 
        # 
        # 
        
        
        # # change all materials to match new material
        # for i in self.entity.prim.Materials:
        #   if(i != submesh):
        #       self.entity.prim.Materials[i] = self.entity.prim.Materials[submesh]
        
    def getNewName(self):
        fooName = 'random'
        return fooName
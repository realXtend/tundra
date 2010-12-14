import circuits
import naali

COMPNAME = "materialtemplate"

class MaterialTemplateHandler(circuits.BaseComponent):
    def __init__(self, entity=None, comp=None, changetype=None):
        circuits.BaseComponent.__init__(self)
        self.entity = entity
        self.comp = comp
        
        naali.library.connect("TextureUrlWasAssigned(uint)", self.newTexture)
        
    def newTexture(self, submesh):
        print "Submesh ", submesh, " got a new texture img"
        print self.entity.prim.Materials[submesh]
        color = '0.5, 0.5, 0.5'
        
        # # get material template from the dynamic component, get material name, use String modulo to create the final material file
        # if (self.comp.GetAttribute('template) is not null):
        #   materialAsString = self.comp.GetAttribute('template')
        #   imageTexTuple = self.entity.prim.Materials[submesh]
        #   materialAsString = materialAsString % {"name": self.getNewName() , "texture": str( imageTexTuple[1] ) }
        #   print templateText
        # # output material file
        # 
        
        
        # # change all materials to match new material
        # for i in self.entity.prim.Materials:
        #   if(i != submesh):
        #       self.entity.prim.Materials[i] = self.entity.prim.Materials[submesh]
        
    def getNewName(self):
        fooName = 'random'
        return fooName
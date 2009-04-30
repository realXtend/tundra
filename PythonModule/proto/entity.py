class Placeable:
    """py mockup of EC_OgrePlaceable.cpp in OgreRender"""
    
    PYNAME = 'place'
    
    def __init__(self):
        self.pos = [0, 0, 0]
        
    def get_x(self):
        return self.pos[0]
    def set_x(self, val):
        self.pos[0] = val
    x = property(get_x, set_x, None, "the x component of the position vector")

class Entity: 
    """py mock for running the test against before implementing in the viewer"""
    def __init__(self):
        self.components = {} #ComponentVector components_; in SceneModule/Entity.h
        
    def add_component(self, comptype):
        self.components[comptype.PYNAME] = comptype()
        
    def __getattr__(self, n):
        return self.components[n]
        
class PlacedEntity(Entity):
    def __init__(self):
        Entity.__init__(self)
        self.add_component(Placeable)
    
    """on the c++ side this is:
    const Foundation::ComponentInterfacePtr &ogre_component = entity->GetComponent("EC_OgrePlaceable");
    OgreRenderer::EC_OgrePlaceable *ogre_pos = dynamic_cast<OgreRenderer::EC_OgrePlaceable *>(ogre_component.get());        
    RexTypes::Vector3 pos((float)sb_pos_x->get_value(), (float)sb_pos_y->get_value(), (float)sb_pos_z->get_value());
    ogre_pos->SetPosition(pos);
    """

def test_move(entity):
    #oldpos = entity.place #or: entity.components["EC_OgrePlaceable"]
    #oops! .. is a ref to the *same* placeable instance
    p = entity.place
    oldx = p.x
    p.x += 1
    assert entity.place.x > (oldx + 0.9)
    print "TEST MOVE SUCCEEDED", entity.place.x, oldx
    
def runtests():
    #e = viewer.scenes['World'].entities[1] #the id for testdummy, or by name?
    #note: now is directly in viewer as GetEntity, defaulting to 'World' scene
    #with mockup now:
    e = PlacedEntity()
    test_move(e)
    
if __name__ == '__main__':
    runtests()
    
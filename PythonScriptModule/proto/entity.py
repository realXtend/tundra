"""
A test / api design for manipulating data in an component of an entity,
and a Python written mockup implementation for an Entity-Component
system that exposes the component data so that the test passes.
This is to plan the py wrapper implementation for the c++ framework,
and possible refactorings there.

Here are three classes, Placeable, Entity and PlacedEntity.
Of Entity there are alternative implementations, one where the 
references to the components are in a separate dict (like they are
in a list in the current c++ impl) and another where the dict 
of the pymodule is used directly, i.e. they are normal members of the entity.

Placeable is a mockup of the the current C++ implementation in
EC_Placeable.cpp.  So far the class is just a position-type 
object that has the get and set methods for each of the x, y and z 
-coordinates. Those three are set as properties so place.x returns 
the x value and place.x = 1 sets it.

PlacedEntity is an entity with a Placeable component.
"""

class Vector3:
    def __init__(self, xyz):
        self.vec = xyz
        
    def get_x(self):
        return self.vec[0]
    def set_x(self, val):
        self.vec[0] = val
    x = property(get_x, set_x, None, "the x component of the position vector")

    def get_y(self):
        return self.vec[1]
    def set_y(self, val):
        self.vec[1] = val
    y = property(get_y, set_y, None, "the y component of the position vector")

    def get_z(self):
        return self.vec[2]
    def set_z(self, val):
        self.vec[2] = val
    z = property(get_z, set_z, None, "the z component of the position vector")


class Placeable:
    """py mockup of EC_Placeable.cpp in OgreRender"""
    
    PYNAME = 'place'
    
    def __init__(self):
        self.pos = Vector3([0, 0, 0])        

class EntityWithSeparateComponentDict:
    """The first implementation, which is close to the current c++ one
    in the sense that it holds a separate component collection"""
    
    def __init__(self):
        """ a dictionary (key-value pair) of this entities components """
        self.components = {} #ComponentVector components_; in SceneModule/Entity.h
        
    def add_component(self, comptype):
        self.components[comptype.PYNAME] = comptype()

    def __getattr__(self, n):
        """ returns the value that corresponds with the key n """
        return self.components[n]
        
    """
    In the current c++ implementation the components that an entity 
    aggregates is a vector/list of the component instances. Here to 
    facilitate exposing the components as normal members within Python,
    the collection is a dictionary (a key-value-pair).
    The __getattr__ method makes it that if you call entity.place, 
    the word 'place' is given as the param 'n' to the method __getattr__ 
    which in turn, returns the value of the key 'n=place' from the dictionary. 
    Due to PYNAME being 'place' in Placeble, the key 'place' returns the aggregated object instance of Placeable, 
    which in turn has x,y,z values in this example, so entity.place.x would
    return the x-coordinate.
    """

class EntityWithComponentsInObjectDict(object): 
    """a more straightforward implementation where the components
    are directly in the dictionary of the python object."""
        
    def add_component(self, comptype):
        """ adding the new component to the dictionary, using the components type as the key and a
            new instance of the component as the value. """
        setattr(self, comptype.PYNAME, comptype())                
 
"""both impls work""" 
#Entity = EntityWithSeparateComponentDict
Entity = EntityWithComponentsInObjectDict

class PlacedEntity(Entity):
    def __init__(self):
        Entity.__init__(self)
        self.add_component(Placeable) #NOTE: we could do the init here, but it will be done in the add_component method
        
class PythonPlacedEntity:
    """
    Note: in normal py we'd just do this. am getting confused how/why this is different,
    but obviously because on the c++ side aggregating arbitary things to a single type
    is not supported directly (the type is static)
    """
    def __init__(self):
        self.place = Placeable()
        
class CurrentCppEntity(object):
    """mimics what there is now in the actual viewer in c(++),
    to help debugging errors with setattr there now"""
    
    def __init__(self):
        self.ec_place = Placeable()
        #print "CppEntity init, place:", self.ec_place
        
    def __getattr__(self, name):
        try:
            return object.__getattr__(self, name)
        except AttributeError:
            pass #print "getting unknown attr"
            
        if name == 'place':
            #print "CppEntity place pos vec:", self.ec_place.pos.vec
            return self.ec_place.pos.vec
            
        raise AttributeError
            
    def __setattr__(self, name, val):
        if name == 'place':
            self.ec_place.pos.vec = val
            return
            
        object.__setattr__(self, name, val)

        
def test_move(entity):
    #oldpos = entity.place #or: entity.components["EC_Placeable"]
    #oops! .. is a ref to the *same* placeable instance
    p = entity.place.pos #first we get the entities place, which is an instance of the Placeable class
    oldx = p.x #then we ask for the x-coordinate from the Placeable
    p.x += 1 #change the x-coordinate
    assert entity.place.pos.x > (oldx + 0.9) #and ,finally test if it actually did move
    print "TEST MOVE SUCCEEDED", entity.place.pos.x, oldx #if we get this far, the move actually worked! Yay.
    
    """on the c++ side this is:
    const Foundation::ComponentInterfacePtr &ogre_component = entity->GetComponent("EC_Placeable");
    EC_Placeable *ogre_pos = dynamic_cast<EC_Placeable *>(ogre_component.get());
    RexTypes::Vector3 pos((float)sb_pos_x->get_value(), (float)sb_pos_y->get_value(), (float)sb_pos_z->get_value());
    ogre_pos->SetPosition(pos);
    """
    
def test_move_cpp(e):
    """copy-pasted from pymodules/test_move.py which is executed within viewer,
    to verify the design against the mockup here now when debugging"""
    p = e.place #.pos - the w.i.p. api has a shortcut now that instead of a placeable with loc,rot,scale it just gives loc now directly
    oldx = p[0] #p.x - Vector3 not wrapped (yet), just gives a tuple
    #p.x += 1 #change the x-coordinate
    newpos = (p[0] + 1, p[1], p[2])
    print "TEST MOVE CPP: trying to move to pos:", newpos
    e.place = newpos
    
    assert e.place[0] > (oldx + 0.9) #and ,finally test if it actually did move
    print "TEST MOVE CPP SUCCEEDED", e.place[0], oldx #if we get this far, the move actually worked! Yay.    
    
def runtests():
    #e = viewer.scenes['World'].entities[1] #the id for testdummy, or by name?
    #note: now is directly in viewer as GetEntity, defaulting to 'World' scene
    #with mockup now:

    e = PlacedEntity()
    #e= PythonPlacedEntity()
    test_move(e)
    
    cppe = CurrentCppEntity()
    test_move_cpp(cppe)
    
if __name__ == '__main__':
    runtests()
    
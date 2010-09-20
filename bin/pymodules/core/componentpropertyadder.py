import circuits
import naali

#the ones not listed here are added using the c++ name, e.g. ent.EC_NetworkPosition
compshorthand = {
    'EC_OgrePlaceable': 'placeable',
    'EC_OgreMesh':'mesh',
    'EC_OgreCamera': 'camera',
    'EC_OgreAnimationController': 'animationcontroller',
    'EC_Highlight': 'highlight',
    'EC_Touchable': 'touchable'
    }   

class ComponentPropertyAdder(circuits.BaseComponent):
    """Adds Naali Entity-Components as Qt Dynamic Properties
    for convenient access from PythonQt and QtScript (js) code."""

    @circuits.handler("on_sceneadded")
    def on_sceneadded(self, name):
        #print "Scene added:", name#,
        s = naali.getScene(name)

        #s.connect("ComponentInitialized(Foundation::ComponentInterface*)", self.onComponentInitialized)
        s.connect("ComponentAdded(Scene::Entity*, IComponent*, AttributeChange::Type)", self.onComponentAdded)

    def onComponentAdded(self, ent, comp, changetype):
        #print "Comp added:", ent, comp, comp.TypeName, comp.Name, changetype
        
        if comp.TypeName in compshorthand:
            propname = compshorthand[comp.TypeName]
        else:
            propname = comp.TypeName

        if propname not in ent.dynamicPropertyNames():
            #first come, first (actually: the only one) served
            #consistent with how inside the c++ side single GetComponent works
            ent.setProperty(propname, comp)

    #XXX \todo: add component removal handling too!

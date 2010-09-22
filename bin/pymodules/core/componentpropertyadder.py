import circuits
import naali
import rexviewer as r
#from PythonQt.QtCore import QVariant
import PythonQt

#the ones not listed here are added using the c++ name, e.g. ent.EC_NetworkPosition
compshorthand = {
    'EC_OgrePlaceable': 'placeable',
    'EC_OgreMesh':'mesh',
    'EC_OgreCamera': 'camera',
    'EC_OgreAnimationController': 'animationcontroller',
    'EC_Highlight': 'highlight',
    'EC_Touchable': 'touchable',
    'EC_AttachedSound': 'sound',
    "EC_OpenSimPresence": 'opensimpresence',
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
        s.connect("ComponentRemoved(Scene::Entity*, IComponent*, AttributeChange::Type)", self.onComponentRemoved)

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

    def onComponentRemoved(self, ent, comp, changetype):
        #r.logInfo("XXX onComponentRemoved called")
        if comp.TypeName in compshorthand:
            propname = compshorthand[comp.TypeName]
        else:
            propname = comp.TypeName
        #r.logInfo("XXX propname " +str(propname))
        #r.logInfo("XXX dynamicpropertynames " + str(ent.dynamicPropertyNames()))
        if propname in ent.dynamicPropertyNames():
            # qt docs: "A property can be removed from an instance by
            # passing the property name and an invalid QVariant value
            # to QObject::setProperty(). The default constructor for
            # QVariant constructs an invalid QVariant."
            #this is probably impossible on py side 'cause we don't see QVariants here, so there's a helper on the c++ side instead.
            #ent.setProperty(propname, invalid_qvariant())
            naali._pythonscriptmodule.RemoveQtDynamicProperty(ent, propname)
            print "XXX deleted prop", propname

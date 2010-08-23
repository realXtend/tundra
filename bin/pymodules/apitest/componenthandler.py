import circuits
import rexviewer as r
import naali

"""a register of component handlers, by type"""
handlertypes = {}
def register(compname, handlertype):
    handlertypes[compname] = handlertype

import animsync
register(animsync.COMPNAME, animsync.AnimationSync)

import door
register(door.COMPNAME, door.DoorHandler)

class ComponenthandlerRegistry(circuits.BaseComponent):
    #def __init__(self):
    #    circuits.BaseComponent.__init__(self)
    #    self.handlerinstances = []

    @circuits.handler("on_sceneadded")
    def on_sceneadded(self, name):
        #print "Scene added:", name#,
        s = naali.getScene(name)

        #s.connect("ComponentInitialized(Foundation::ComponentInterface*)", self.onComponentInitialized)
        s.connect("ComponentAdded(Scene::Entity*, Foundation::ComponentInterface*, AttributeChange::Type)", self.onComponentAdded)

    #def onComponentInitialized(self, comp):
    #    print "Comp inited:", comp

    def onComponentAdded(self, entity, comp, changetype):
        #print "Comp added:", entity, comp, changetype
        #print comp.className()
        if comp.className() == "EC_DynamicComponent":
            print "comp Name:", comp.Name
            if comp.Name in handlertypes:
                handlertypes[comp.Name](entity, comp, changetype)
                #self.handlerinstances.append(h)

"""
A test / draft for defining Naali Entity-Components with Naali Attribute data.
"""

import circuits
import rexviewer as r
import naali

class DynamiccomponentHandler(circuits.Component):
    def __init__(self):
        circuits.Component.__init__(self)
        obid = None #2514504734 #1220426476
        self.d = None
        if obid is not None:
            ent = r.getEntity(obid)
            self.add_component(ent)

    def on_sceneadded(self, name):
        print "Scene added:", name#,
        s = naali.getScene(name)
        print s
        print dir(s)

        #s.connect("ComponentInitialized(Foundation::ComponentInterface*)", self.onComponentInitialized)
        s.connect("ComponentAdded(Scene::Entity*, Foundation::ComponentInterface*, Foundation::ChangeType)", self.onComponentAdded)

    #def onComponentInitialized(self, comp):
    #    print "Comp inited:", comp

    def onComponentAdded(self, entity, comp, changetype):
        print "Comp added:", entity, comp, changetype
        #print comp.className()
        if comp.className() == "EC_DynamicComponent":
            comp.connect("OnChanged()", self.onChanged)
            self.d = comp

    def add_component(self, ent):
        try:
            ent.dynamic
        except AttributeError:
            ent.createComponent("EC_DynamicComponent")
        print ent.dynamic
            
        #d = ent.dynamic
        #print dir(d)
        #d.AddAttribute()
        #print d.GetAttribute()

        #d.connect("OnChanged()", self.onChanged)
        
        #self.d = d

    def onChanged(self):
        print "onChanged",
        d = self.d
        if d is not None:
            print d.GetAttribute()
        else:
            print "- don't know what :o"

    def update(self, frametime):
        d = self.d
        if d is not None:
            if 0:
                avid = r.getUserAvatarId()
                e = r.getEntity(avid)
                pos = e.placeable.Position
                print pos
                d.SetAttribute(pos.x())
               #d.ComponentChanged(1) #Foundation::ChangeType::Local - not a qenum yet
            #print d.GetAttribute()

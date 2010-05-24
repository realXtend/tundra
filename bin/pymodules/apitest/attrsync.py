"""
A test / draft for defining Naali Entity-Components with Naali Attribute data.
"""

import circuits
import rexviewer as r

class DynamiccomponentHandler(circuits.Component):
    def __init__(self):
        circuits.Component.__init__(self)
        obid = None #2425458104
        self.d = None
        if obid is not None:
            ent = r.getEntity(obid)
            self.add_component(ent)

    def add_component(self, ent):
        #try:
        #    ent.dynamic
        #except AttributeError:
        #    ent.createComponent("EC_DynamicComponent")
        #print ent.dynamic
            
        d = ent.dynamic
        print dir(d)
        d.AddAttribute()
        print d.GetAttribute()
        
        self.d = d

    def update(self, frametime):
        d = self.d
        if d is not None:
            if 1:
                d.SetAttribute(0.2)
                #d.ComponentChanged(1) #Foundation::ChangeType::Local - not a qenum yet
            print d.GetAttribute()

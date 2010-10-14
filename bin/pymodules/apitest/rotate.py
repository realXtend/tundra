import circuits
from PythonQt.QtGui import QQuaternion as Quat
from PythonQt.QtGui import QVector3D as Vec
import naali

COMPNAME = "rotation"

class RotationHandler(circuits.BaseComponent):
    def __init__(self, entity=None, comp=None, changetype=None):
        circuits.BaseComponent.__init__(self)
        self.entity = entity
        self.comp = comp
        if self.comp is not None: #normal run, check for nonEC run now
            # Todo: OnChanged() is deprecated
            comp.connect("OnChanged()", self.onChanged)
        self.rot = Quat.fromAxisAndAngle(Vec(0, 1, 0), 1)

    def onChanged(self):
        y = self.comp.GetAttribute('y')
        self.rot = Quat.fromAxisAndAngle(Vec(0, y, 0), 1)
        #print self.rot, y
        
    @circuits.handler("update")
    def update(self, frametime):
        if self.entity is not None:
            p = self.entity.placeable
            ort = p.Orientation
            ort *= self.rot
            p.Orientation = ort

        # else: #testing without EC, as a autoloaded module
        #     entid = 2088826547
        #     try:
        #         self.entity = naali.getEntity(entid)
        #     except:
        #         pass #not there (yet)
        #     else:
        #         self.entity.createComponent("EC_DynamicComponent")
        #         oldent = r.getEntity(ent.id)
        #         self.comp = oldent.dynamic

    @circuits.handler("on_logout")
    def on_logout(self, evid):
        self.entity = None #XXX figure out proper unregistering, preferrably in componenthandler.py / EC_Script biz

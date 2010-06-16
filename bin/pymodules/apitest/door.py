import json

from PythonQt import QtGui
from PythonQt.QtGui import QVector3D as Vec
from PythonQt.QtGui import QGroupBox, QVBoxLayout, QPushButton

import rexviewer as r

from componenthandler import DynamiccomponentHandler

OPENPOS = Vec(127, 128, 30)
CLOSEPOS = Vec(127, 127, 30)

#opened = AttributeBoolean()
#locked = AttributeBoolean()

"""had the door as a class with json serialization first
- works, but unnecessarily complex, 'cause it's now just two bools
class DoorState:
    def __init__(self, opened=False, locked=True):
        self.opened = opened
        self.locked = locked

    def __str__(self):
        return "%s - %s" % (self.opened, self.locked)

class DoorEncoder(json.JSONEncoder):
    def default(self, obj):
        if isinstance(obj, DoorState):
            return {'opened': obj.opened, 'locked': obj.locked}
        return json.JSONEncoder.default(self, obj)
def as_door(dct):
    return DoorState(dct['opened'], dct['locked'])
"""

"""the synched attribute data - this is initial state if none comes from server
not here anymore to not mess the currently shared data with other comps"""
#doorinit = {'opened': False,
#            'locked': True}

class DoorHandler(DynamiccomponentHandler):
    GUINAME = "Door Handler"
    inworld_inited = False #a cheap hackish substitute for some initing system

    def onChanged(self):
        ent = r.getEntity(self.comp.GetParentEntityId())

        if not self.inworld_inited:
            #if hasattr(self.comp, 'touchable')
            try:
                t = ent.touchable
            except AttributeError:
                print "no touchable in door? it doesn't persist yet? adding..", ent.id
                print ent.createComponent("EC_Touchable")
                t = ent.touchable
            else:
                print "touchable pre-existed in door."
            t.connect('Clicked()', self.open)
            self.inworld_inited = True

        if self.proxywidget is None and self.widget is not None:
            print "Door DynamicComponent handler registering to GUI"
            self.registergui()

        data = self.comp.GetAttribute()
        print "GetAttr got:", data
        try:
            self.door = json.loads(data) #, object_hook = as_door)
        except ValueError:
            print "not valid door data in attr json - using default DoorState"            
            self.door = doorinit #DoorState()

        newpos = OPENPOS if self.door['opened'] else CLOSEPOS
        ent.placeable.Position = newpos        
        print self.door, ent.placeable.Position

        self.openbut.text = "Close" if self.door['opened'] else "Open"
        self.lockbut.text = "Unlock" if self.door['locked'] else "Lock"
        if self.door['locked'] and not self.door['opened']:
            self.openbut.enabled = False
        else:
            self.openbut.enabled = True
        
        """NOTE:
        this code doesn't now sync the normal ob pos to server
        at all. the object is moved in all clients only.
        when logging back to a server, wasn't seeing the right positions,
        probably because server send pos update after the comp sync."""
        self.forcepos = ent.placeable.Position
        
    def initgui(self):
        #qt widget ui
        group = QGroupBox()
        box = QVBoxLayout(group)

        self.openbut = QPushButton("init", group)
        self.lockbut = QPushButton("init", group)
        box.addWidget(self.openbut)
        box.addWidget(self.lockbut)

        self.openbut.connect('clicked()', self.open)
        self.lockbut.connect('clicked()', self.lock)

        self.widget = group
        self.forcepos = None

    def open(self):
        print "open"
        #was when abusing float as a bool:
        #oldval = self.door.opened #comp.GetAttribute()
        #newval = 0 if oldval else 1
        #self.comp.SetAttribute(newval)

        if self.door['opened'] or not self.door['locked']:
            self.door['opened'] = not self.door['opened']
            self.sync()
        else:
            print "Can't open a locked door!"

    def lock(self):
        #\todo if has key
        self.door['locked'] = not self.door['locked']
        self.sync()

    def sync(self):
        if self.comp is not None:
            newdata = json.dumps(self.door) #, cls=DoorEncoder)
            print "setting Door data to:", newdata
            self.comp.SetAttribute(newdata)

    def update(self, t):
        if self.forcepos is not None:
            ent = r.getEntity(self.comp.GetParentEntityId())
            ent.placeable.Position = self.forcepos

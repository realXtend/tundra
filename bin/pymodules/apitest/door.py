from PythonQt import QtGui
from PythonQt.QtGui import QVector3D as Vec
import rexviewer as r

from componenthandler import DynamiccomponentHandler

OPENPOS = Vec(127, 128, 30)
CLOSEPOS = Vec(127, 127, 30)

#opened = AttributeBoolean()
#locked = AttributeBoolean()

class DoorHandler(DynamiccomponentHandler):
    GUINAME = "Door Handler"

    def onChanged(self):
        ent = r.getEntity(self.comp.GetParentEntityId())
        opened = self.comp.GetAttribute()
        newpos = OPENPOS if opened else CLOSEPOS
        ent.placeable.Position = newpos        
        print opened, ent.placeable.Position

        """NOTE:
        this code doesn't now sync the normal ob pos to server
        at all. the object is moved in all clients only.
        when logging back to a server, wasn't seeing the right positions,
        probably because server send pos update after the comp sync."""
        self.forcepos = ent.placeable.Position

    def initgui(self):
        self.widget = QtGui.QPushButton()
        self.widget.connect('clicked()', self.clicked)

        self.forcepos = None

    def clicked(self):
        if self.comp is not None:
            #self.comp = not self.comp
            oldval = self.comp.GetAttribute()
            newval = 0 if oldval else 1
            self.comp.SetAttribute(newval)

    def update(self, t):
        if self.forcepos is not None:
            ent = r.getEntity(self.comp.GetParentEntityId())
            ent.placeable.Position = self.forcepos

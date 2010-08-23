import json

from PythonQt import QtGui
from PythonQt.QtGui import QVector3D as Vec
from PythonQt.QtGui import QGroupBox, QVBoxLayout, QPushButton

import rexviewer as r

import circuits #for @handler
from componenthandler import DynamiccomponentHandler

#should be in the EC data
OPENPOS = Vec(101.862, 82.6978, 24.9221)
CLOSEPOS = Vec(99.65, 82.6978, 24.9221)

"""for changing the cursor on hover"""
import PythonQt
qapp = PythonQt.Qt.QApplication.instance()
import PythonQt.QtGui as gui
cursor = gui.QCursor()
#print cursor, cursor.shape()
def setcursor(ctype):
    cursor.setShape(ctype)    
    qapp.setOverrideCursor(cursor)

class DoorHandler(DynamiccomponentHandler):
    GUINAME = "Door Handler"
    COMPNAME = "door" #the DC name identifier string that this handler looks for
    ADDMENU = True

    def __init__(self):
        DynamiccomponentHandler.__init__(self)
        self.inworld_inited = False #a cheap hackish substitute for some initing system
        self.hovering = 0

    def onChanged(self):
        print "door data changed"
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
            t.connect('MouseHover()', self.hover)
            self.inworld_inited = True

        if self.proxywidget is None and self.widget is not None:
            print "Door DynamicComponent handler registering to GUI"
            self.registergui()

        opened = self.opened
        locked = self.locked

        newpos = OPENPOS if opened else CLOSEPOS
        ent.placeable.Position = newpos        
        print opened, type(opened), ent.placeable.Position

        self.openbut.text = "Close" if opened else "Open"
        self.lockbut.text = "Unlock" if locked else "Lock"
        if locked and not opened:
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

        #group.show() #as a temp workaround as naali ui system is borked XXX

    def get_opened(self):
        if self.comp is not None:
            return self.comp.GetAttribute("opened")
        else:
            return None
    def set_opened(self, newval):
        self.comp.SetAttribute("opened", newval)
        self.comp.OnChanged()
    opened = property(get_opened, set_opened)

    def get_locked(self):
        if self.comp is not None:
            return self.comp.GetAttribute("locked")
        else:
            return None
    def set_locked(self, newval):
        self.comp.SetAttribute("locked", newval)
        self.comp.OnChanged()
    locked = property(get_locked, set_locked)

    def open(self):
        print "open"

        if self.opened or not self.locked:
            self.opened = not self.opened
            print self.opened
        else:
            print "Can't open a locked door!"

    def lock(self):
        #\todo if has key
        self.locked = not self.locked

    def hover(self):
        #XXX add locked check too
        ctype = 1 if self.opened else 2
        setcursor(ctype)
        self.hovering = 10 #a dirty hack to do onMouseExit (this is onMouseOver). not 100% reliable it seems XXX

    @circuits.handler("update")
    def update(self, t):
        if self.forcepos is not None:
            ent = r.getEntity(self.comp.GetParentEntityId())
            ent.placeable.Position = self.forcepos

        if self.hovering > 0:
            self.hovering -= 1
            if self.hovering == 0:
                setcursor(0)

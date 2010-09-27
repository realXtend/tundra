from PythonQt import QtGui
from PythonQt.QtGui import QVector3D as Vec
from PythonQt.QtGui import QGroupBox, QVBoxLayout, QPushButton

import rexviewer as r #only for uiscenemanager now - make that to use naali api too

#componenthandlers don't necessarily need to be naali modules,
#but this one needs to listen to update events to do the forcepos hack
import circuits

#should be in the EC data
OPENPOS = Vec(101.862, 82.6978, 24.9221)
CLOSEPOS = Vec(99.65, 82.6978, 24.9221)

"""for changing the cursor on hover"""
import PythonQt
qapp = PythonQt.Qt.QApplication.instance()
import PythonQt.QtGui as gui
cursor = gui.QCursor()

def setcursor(ctype):
    cursor.setShape(ctype)
    current = qapp.overrideCursor()
    if current != None:
        if current.shape() != ctype:
            qapp.setOverrideCursor(cursor)
    else:
        qapp.setOverrideCursor(cursor)

COMPNAME = "door" #the DC name identifier string that this handler looks for

class DoorHandler(circuits.BaseComponent):
    GUINAME = "Door Handler"
    ADDMENU = True

    def __init__(self, entity, comp, changetype):
        self.comp = comp
        circuits.BaseComponent.__init__(self)

        comp.connect("OnChanged()", self.onChanged)
        self.inworld_inited = False #a cheap hackish substitute for some initing system
        self.initgui()

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

        #naali proxywidget boilerplate
        uism = r.getUiSceneManager()
        self.proxywidget = r.createUiProxyWidget(self.widget)
        self.proxywidget.setWindowTitle(self.GUINAME)
        if not uism.AddWidgetToScene(self.proxywidget):
            print "Adding the ProxyWidget to the bar failed."
        uism.AddWidgetToMenu(self.proxywidget, self.GUINAME, "Developer Tools")

    def onChanged(self):
        try:
            ent = self.comp.GetParentEntity()
        except ValueError: #the entity has been removed or something
            return

        print "door data changed"

        if not self.inworld_inited:
            #if hasattr(self.comp, 'touchable')
            try:
                t = ent.touchable
            except AttributeError:
                print "no touchable in door? it doesn't persist yet? adding..", ent.Id
                t = ent.GetOrCreateComponentRaw("EC_Touchable")
            else:
                print "touchable pre-existed in door."
            t.connect('MousePressed()', self.open)
            t.connect('MouseHoverIn()', self.hover_in)
            t.connect('MouseHoverOut()', self.hover_out)
            self.inworld_inited = True

        opened = self.opened
        locked = self.locked

        newpos = OPENPOS if opened else CLOSEPOS
        ent.placeable.Position = newpos        
        #print opened, type(opened), ent.placeable.Position

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
        #print "open"
        if self.opened or not self.locked:
            self.opened = not self.opened
            print self.opened
        else:
            print "Can't open a locked door!"

    def lock(self):
        #\todo if has key
        self.locked = not self.locked

    def hover_in(self):
        #XXX add locked check too
        ctype = 1 if self.opened else 2
        setcursor(ctype)

    def hover_out(self):
        curr_cursor = qapp.overrideCursor()
        while curr_cursor != None:
            qapp.restoreOverrideCursor()
            curr_cursor = qapp.overrideCursor()

    @circuits.handler("update")
    def update(self, t):
        try:
            ent = self.comp.GetParentEntity()
        except ValueError: #the entity has been removed or something
            return # nothing useful to do anyway

        if self.forcepos is not None:
            ent.placeable.Position = self.forcepos

    @circuits.handler("on_logout")
    def removegui(self, evid):
        self.proxywidget.hide()
        uism = r.getUiSceneManager()
        uism.RemoveWidgetFromMenu(self.proxywidget)
        uism.RemoveWidgetFromScene(self.proxywidget)
        

"""
A test / draft for defining Naali Entity-Components with Naali Attribute data.
This one synchs animation state with a GUI slider.
There is now also another test, door.py, so moved common parts to componenthandler.py

Originally this used the first test version of DynamicComponent that provided a single float attribute (Attribute<float>).
That can be useful when testing performance and robustness of an upcoming ECA sync mechanism.
The current implementation in Naali 0.2 uses a workaround that needed no protocol nor server changes to old ReX,
where all attributes of all components for a single entity are synched and stored as xml.

After this test, a Door test with two attributes: opened & locked, was written.
First as door.py here, and then as door.js that can be safely loaded from the net (the source is for convenience in Naali repo jsmodules).
To add support for multiple attributes of various types easily to py & js, that was made using a single Naali string attr to store json.
With the current ECA sync the performance makes no diff, so there are little drawbacks in this shortcut to get to experiment more.

Currently the DynamicComponent system doesn't differentiate/allow multiple instances for a single entity
-- that will be easy to add using the existing (quite new) Component instance identification system in Naali.
But to allow this to work simultaneously with the door tests, adopting this to use the current single JSON attr also.
Also, the initial idea was to have the door handler sync animation state of the door (for a skel animated folding door (haitariovi)),
so perhaps these merge somehow.
"""

from __future__ import division
import time
import rexviewer as r

import PythonQt
from PythonQt import QtGui, QtCore

INTERVAL = 0.2

class AnimationSync:
    GUINAME = "Animation Sync"

    def __init__(self, entity, comp, changetype):
        comp.connect("OnChanged()", self.onChanged)
        self.comp = comp
        self.inworld_inited = False #a cheap hackish substitute for some initing system
        self.widget = QtGui.QSlider(QtCore.Qt.Horizontal)
        self.widget.connect('valueChanged(int)', self.sliderChanged)

        #naali proxywidget boilerplate
        uism = r.getUiSceneManager()
        self.proxywidget = r.createUiProxyWidget(self.widget)
        self.proxywidget.setWindowTitle(self.GUINAME)
        if not uism.AddWidgetToScene(self.proxywidget):
            print "Adding the ProxyWidget to the bar failed."
        #uism.AddWidgetToMenu(self.proxywidget, self.GUINAME, "Developer Tools")

        #to not flood the network
        self.prev_sync = 0
        
    #json serialization to port to work with how DC works now with door.py & door.js
    def sliderChanged(self, guival):
        comp = self.comp
        if comp is not None:
            now = time.time()
            if self.prev_sync + INTERVAL < now:
                comp.SetAttribute("timepos", guival / 100)
                comp.OnChanged() #XXX change to OnAttributeChanged when possible
                self.prev_sync = now

    def onChanged(self):
        v = self.comp.GetAttribute('timepos')

        #copy-paste from door.py which also had a onClick handler
        if not self.inworld_inited:
            ent = r.getEntity(self.comp.GetParentEntityId())
            try:
                t = ent.touchable
            except AttributeError:
                print "no touchable in animsynced obj? it doesn't persist yet? adding..", ent.id
                print ent.createComponent("EC_Touchable")
                t = ent.touchable
            else:
                print "touchable pre-existed in animated character for animsync."
            t.connect('Clicked()', self.showgui)
            self.inworld_inited = True        

        if self.proxywidget is None and self.widget is not None:
            #if self.widget is None:
            #    self.initgui()
            print "AnimSync DynamicComponent handler registering to GUI"
            self.registergui()

        #print "onChanged",
        comp = self.comp
        if comp is not None:
            #print comp.GetAttribute()
            ent = r.getEntity(comp.GetParentEntityId())
            try:
                a = ent.animationcontroller
            except AttributeError:
                print "ent with DynamicComponent doesn't have animation"
                return

            #print a
            a.SetAnimationTimePosition("Wave", v)
            self.widget.value = v * 100 #needs changetypes to work well, i guess
            
        else:
            print "- don't know what :o"

    def showgui(self):
        self.proxywidget.show()

COMPNAME = "animsync"
#now done in componenthandler 'cause this is not a circuits component / naali module
#componenthandler.register(COMPNAME, AnimationSync)
    
    #failed in the attempt to hide the tool when go out of scenes that have animsync - something of the widget stays there, so get error prints QPainter::end: Painter not active, aborted
    #def on_logout(self, idt):
    #    if self.proxywidget is not None:
    #        self.proxywidget.hide()
    #        uism = r.getUiSceneManager()
    #        uism.RemoveProxyWidgetFromScene(self.proxywidget)
    #        self.proxywidget = None
    #        self.widget = None

#    def update(self, frametime):
#        comp = self.comp
#        if comp is not None:
#            if 0:
#                avid = r.getUserAvatarId()
#                e = r.getEntity(avid)
#                pos = e.placeable.Position
#                print pos
#                comp.SetAttribute(pos.x())
               #d.ComponentChanged(1) #Foundation::ChangeType::Local - not a qenum yet
            #print d.GetAttribute()

"""
gui stuff that used to be here when this was a baseclass
    @circuits.handler("on_logout")
    def on_logout(self, idt):
        if self.comp is not None:
            try:
                self.comp.disconnect("OnChanged()", self.onChanged)
            finally: #disconnect fails if the entity had been deleted
                self.comp = None

    @circuits.handler("on_exit")
    def on_exit(self):
        if self.proxywidget is not None:
            uism = r.getUiSceneManager()
            uism.RemoveWidgetFromScene(self.proxywidget)

        self.widget = None
        self.proxywidget = None
        self.initgui()

    def initgui(self):
        pass #overridden in subclasses

    def registergui(self):
"""

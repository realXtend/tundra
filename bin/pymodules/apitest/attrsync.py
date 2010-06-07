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
import json

import PythonQt
from PythonQt import QtGui, QtCore

from componenthandler import DynamiccomponentHandler

INTERVAL = 0.2

class AnimationSync(DynamiccomponentHandler):
    GUINAME = "Animation Sync"

    def initgui(self):
        self.widget = QtGui.QSlider(QtCore.Qt.Horizontal)
        self.widget.connect('valueChanged(int)', self.sliderChanged)

        #to not flood the network
        self.prev_sync = 0
        
    #json serialization to port to work with how DC works now with door.py & door.js
    def getdata(self):
        data = self.comp.GetAttribute()
        d = json.loads(data)
        return d
    
    def setval(self, val):
        d = self.getdata()
        d['animpos'] = val
        self.comp.SetAttribute(json.dumps(d))
        
    def sliderChanged(self, guival):
        #print guival
        comp = self.comp
        if comp is not None:
            now = time.time()
            if self.prev_sync + INTERVAL < now:
                #was with the float attr:
                #comp.SetAttribute(val / 100)
                self.setval(guival / 100)
                self.prev_sync = now

    def onChanged(self):
        d = self.getdata()
        if 'animpos' in d:
            v = d['animpos']
        else: #no data for this handler
            return

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

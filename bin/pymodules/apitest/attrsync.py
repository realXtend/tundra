"""
A test / draft for defining Naali Entity-Components with Naali Attribute data.
This one synchs animation state with a GUI slider.
There is now also another test, door.py, so moved common parts to componenthandler.py
"""

from __future__ import division
import time
import rexviewer as r

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

    def sliderChanged(self, val):
        print val
        comp = self.comp
        if comp is not None:
            now = time.time()
            if self.prev_sync + INTERVAL < now:
                comp.SetAttribute(val / 100)
                self.prev_sync = now

    def onChanged(self):
        print "onChanged",
        comp = self.comp
        if comp is not None:
            print comp.GetAttribute()
            ent = r.getEntity(comp.GetParentEntityId())
            try:
                a = ent.animationcontroller
            except AttributeError:
                print "ent with DynamicComponent doesn't have animation"
                return

            #print a
            v = comp.GetAttribute()
            a.SetAnimationTimePosition("Wave", v)
            #self.widget.value = v * 100 #needs changetypes to work well, i guess
            
        else:
            print "- don't know what :o"

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

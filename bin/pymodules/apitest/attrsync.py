"""
A test / draft for defining Naali Entity-Components with Naali Attribute data.
"""
from __future__ import division
import time

import circuits
import rexviewer as r
import naali

import PythonQt
from PythonQt import QtGui, QtCore

INTERVAL = 0.2

class DynamiccomponentHandler(circuits.Component):
    def __init__(self):
        circuits.Component.__init__(self)
        obid = None 
        #obid = 1136641184
        #obid = 1239367367
        self.d = None
        if obid is not None:
            ent = r.getEntity(obid)
            self.add_component(ent)

        self.widget = QtGui.QSlider(QtCore.Qt.Horizontal)
        self.widget.connect('valueChanged(int)', self.sliderChanged)

        uism = r.getUiSceneManager()
        uiprops = r.createUiWidgetProperty(1)
        uiprops.widget_name_ = "DynamicComponent Widget"
        self.proxywidget = r.createUiProxyWidget(self.widget, uiprops)
        if not uism.AddProxyWidget(self.proxywidget):
            print "Adding the ProxyWidget to the bar failed."

        #to not flood the network
        self.prev_sync = 0

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
            print "DYNAMIC COMPONENT FOUND", self.d

    def add_component(self, ent):
        try:
            ent.dynamic
        except AttributeError:
            ent.createComponent("EC_DynamicComponent")
        print ent.dynamic
            
        if 1:
            d = ent.dynamic
            #print dir(d)
            #d.AddAttribute()
            print d.GetAttribute()

            d.connect("OnChanged()", self.onChanged)
            self.d = d

    def sliderChanged(self, val):
        print val
        d = self.d
        if d is not None:
            now = time.time()
            if self.prev_sync + INTERVAL < now:
                d.SetAttribute(val / 100)
                self.prev_sync = now

    def onChanged(self):
        print "onChanged",
        d = self.d
        if d is not None:
            print d.GetAttribute()
            ent = r.getEntity(d.GetParentEntityId())
            try:
                a = ent.animationcontroller
            except AttributeError:
                print "ent with DynamicComponent doesn't have animation"
                return

            #print a
            v = d.GetAttribute()
            a.SetAnimationTimePosition("Wave", v)
            #self.widget.value = v * 100 #needs changetypes to work well, i guess
            
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

    def on_exit(self):
        uism = r.getUiSceneManager()
        uism.RemoveProxyWidgetFromScene(self.proxywidget)
    

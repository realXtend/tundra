#!/usr/bin/python

import loader

import PythonQt.QtGui
Vec = PythonQt.QtGui.QVector3D
Quat = PythonQt.QtGui.QQuaternion


class DotSceneManager:
    """ stores the scene nodes with naali entities for further manipulation """
    
    def __init__(self):
        self.nodes = {}
        self.flipZY = False
        
    def setPosition(self, x, y ,z):
        """ set new position diff for nodes """
        for k, oNode in self.nodes.iteritems():
            #print "set position for", k
            self.setOgreNodePosition(oNode, x, y ,z)
        
    def setScale(self, x, y, z):
        """ set x, y, z scale fro objects """
        for k, oNode in self.nodes.iteritems():
            #print "set scale for", k
            self.setOgreNodeScale(oNode, x, y ,z)
        
    def test(self):
        print "test"
        
    def setOgreNodePosition(self, on, x, y ,z):
        e = on.naali_ent
        p = e.placeable
        onx = on.position.x()
        ony = on.position.y()
        onz = on.position.z()
        if(self.flipZY):
            p.Position = Vec(x + onx, z + onz, y + ony)
        else:
            p.Position = Vec(x + onx, y + ony, z + onz)
        pass
        
    def setOgreNodeScale(self, on, x, y ,z):
        e = on.naali_ent
        p = e.placeable
        onx = on.scale.x()
        ony = on.scale.y()
        onz = on.scale.z()
        if(self.flipZY):
            p.Scale = Vec(x * onx, z * onz, y * ony)
        else:
            p.Scale = Vec(x * onx, y * ony, z * onz)
        pass
        
    def setFlipZY(self, enabled, xsift, ysift, zsift, xscale, yscale, zscale):
        self.flipZY = enabled
        for k, oNode in self.nodes.iteritems():
            self.setOgreNodePosition(oNode, xsift, ysift, zsift)
            self.setOgreNodeScale(oNode, xscale, yscale, zscale)
        
    def setHighlight(self, enabled):
        if(enabled):
            self.highlight()
        else:
            self.undoHighlight()
        
    def highlight(self):
        for k, oNode in self.nodes.iteritems():
            e = oNode.naali_ent
            try:
                e.highlight
            except AttributeError:
                e.GetOrCreateComponentRaw("EC_Highlight")
            h = e.highlight
            if not h.IsVisible():
                h.Show()
            else:
                print "objectedit.highlight called for an already hilited entity: %d" % ent.id

    def undoHighlight(self):
        for k, oNode in self.nodes.iteritems():
            e = oNode.naali_ent
            try:
                e.highlight
            except AttributeError:
                e.GetOrCreateComponentRaw("EC_Highlight")
            h = e.highlight
            e.RemoveComponentRaw(h)

#!/usr/bin/python

import loader
from math import sqrt

import PythonQt.QtGui
Vec = PythonQt.QtGui.QVector3D
Quat = PythonQt.QtGui.QQuaternion


class DotSceneManager:
    """ stores the scene nodes with naali entities for further manipulation """
    
    def __init__(self):
        self.nodes = {}
        #self.flipZY = True
        self.flipZY = False
        self.initialized=False
        self.centerPoint = None
        self.nodeCenterVectors={} # node:vector pairs
        self.xshift = 127
        self.yshift = 127
        self.zshift = 25
        
    def setPosition(self, x, y ,z):
        """ set new position diff for nodes """
        self.xshift = x
        self.yshift = y
        self.zshift = z
        for k, oNode in self.nodes.iteritems():
            ## print "set position for", k
            self.setOgreNodePosition(oNode, x, y ,z)
        
    def setScale(self, x, y, z):
        """ set x, y, z scale fro objects """
        for k, oNode in self.nodes.iteritems():
            ## print "set scale for", k
            self.setOgreNodeScale(oNode, x, y ,z)
        
    def test(self):
        # print "test"
        pass
        
    def setOgreNodePosition(self, on, x, y ,z):
        e = on.naali_ent
        p = e.placeable
        onx = on.position.x()
        ony = on.position.y()
        onz = on.position.z()
        p.Position = Vec(x + onx, y + ony, z + onz)
        # print p.Position
        # if(self.flipZY):
            # p.Position = Vec(x + onx, z + onz, y + ony)
        # else:
            # p.Position = Vec(x + onx, y + ony, z + onz)
        # pass
        
    def setOgreNodeScale(self, on, x, y ,z):
        e = on.naali_ent
        p = e.placeable
        onx = on.scale.x()
        ony = on.scale.y()
        onz = on.scale.z()
        p.Scale = Vec(x * onx, y * ony, z * onz)
        # if(self.flipZY):
            # p.Scale = Vec(x * onx, z * onz, y * ony)
        # else:
            # p.Scale = Vec(x * onx, y * ony, z * onz)
        # pass
        
    # def setFlipZY(self, enabled, xshift, yshift, zshift, xscale, yscale, zscale):
        # self.flipZY = enabled
        # for k, oNode in self.nodes.iteritems():
            # self.setOgreNodePosition(oNode, xshift, yshift, zshift)
            # self.setOgreNodeScale(oNode, xscale, yscale, zscale)
            
    def setFlipZY(self, enabled):
        oldVal = self.flipZY
        # print "oldVal, enbled--------------------"
        # print oldVal, enabled
        self.flipZY = enabled
        
        for k, oNode in self.nodes.iteritems():
            self.setOgreNodePosition(oNode, self.xshift, self.yshift, self.zshift)
            #self.setOgreNodeScale(oNode, xscale, yscale, zscale)

        if(oldVal==enabled):
            return
        if(oldVal==True and enabled==False):
            self.rotateScene90DegreesAlongAxis('x') # rotate back
        else:
            self.rotateScene90DegreesAlongAxis('-x') # rotate 
            
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
                # print "objectedit.highlight called for an already hilited entity: %d" % ent.id
                pass

    def undoHighlight(self):
        for k, oNode in self.nodes.iteritems():
            e = oNode.naali_ent
            try:
                e.highlight
            except AttributeError:
                e.GetOrCreateComponentRaw("EC_Highlight")
            h = e.highlight
            e.RemoveComponentRaw(h)

    
    def setCenterPointAndCenterVectors(self):
        # center point
        if(self.nodes.__len__()>1):
            sumVec = Vec(0,0,0)
            for k, oNode in self.nodes.iteritems():
                sumVec = self.vectorAdd(sumVec, oNode.position)
            sumX = sumVec.x()
            sumY = sumVec.y()
            sumZ = sumVec.z()
            centerX = sumX/float(self.nodes.__len__())
            centerY = sumY/float(self.nodes.__len__())
            centerZ = sumZ/float(self.nodes.__len__())
            relativeCenter = Vec(centerX, centerY, centerZ)
            self.centerPoint = Vec(centerX+self.xshift, centerY + self.yshift, centerZ + self.zshift)
            # print "center point: ", self.centerPoint.x(), self.centerPoint.y(), self.centerPoint.z()
            # update shift
            self.xshift = self.centerPoint.x()
            self.yshift = self.centerPoint.y()
            self.zshift = self.centerPoint.z()
            
            # center Vecs
            for k, oNode in self.nodes.iteritems():
                # print "oNode.pos"
                # print oNode.position
                # print "relativeCenter"
                # print relativeCenter
                diffVec = self.vectorDifference(oNode.position, relativeCenter)
                # print "new Center Vec"
                # print diffVec
                oNode.position = diffVec
                self.nodeCenterVectors[oNode]=diffVec
        elif(self.nodes.__len__()==1):
            oNode=self.nodes[self.nodes.keys()[0]]
            self.centerPoint=oNode.position
            
        
    def vectorAdd(self, v1, v2):
        """ alternative for just doin Vec1 + Vec2, that had weird results """
        x = v1.x()+v2.x()
        y = v1.y()+v2.y()
        z = v1.z()+v2.z()
        return Vec(x, y, z)

    def vectorDifference(self, v1, v2):
        """ alternative for just doin Vec1 - Vec2, that has weird results """
        x = v1.x()-v2.x()
        y = v1.y()-v2.y()
        z = v1.z()-v2.z()
        return Vec(x, y, z)

    def rotateScene90DegreesAlongAxis(self, axis):
        #if not self.initialized: do this everytime
        self.setCenterPointAndCenterVectors()
        #rotate CenterVecs and set new positions, + rotate each node
        switch={'x': Quat(1,1,0,0), 'y': Quat(1,0,1,0), 'z': Quat(1,0,0,1), '-x': Quat(1,-1,0,0), '-y': Quat(1,0,-1,0), '-z': Quat(1,0,0,-1)}
        q = switch[axis]
        
        # print "rotations:"
        
        if(self.nodes.__len__()>1): # only do rotations for locations if node amount > 1        
            for oNode, CVec in self.nodeCenterVectors.iteritems():
                CRot=q.rotatedVector(CVec)
                calibVec = self.calibrateVec(CRot, CVec)
                # print calibVec
                self.nodeCenterVectors[oNode]=calibVec
            # set new Positions
            for k, oNode in self.nodes.iteritems():
                centerVec = self.nodeCenterVectors[oNode]
                newPos = self.vectorAdd(self.centerPoint, centerVec)
                # print "center point: ", self.centerPoint.x(), self.centerPoint.y(), self.centerPoint.z()
                # print "center vec: ", centerVec.x(), centerVec.y(), centerVec.z()
                # print "old pos: ", oNode.position.x(), oNode.position.y(), oNode.position.z()
                # print "set to new position: ", newPos.x(), newPos.y() ,newPos.z()
                # print "shift: ", self.xshift, self.yshift, self.zshift
                newNodePos = self.vectorDifference(newPos, Vec(self.xshift, self.yshift, self.zshift))
                # print "new node pos:", newNodePos
                oNode.position=newNodePos
                self.setOgreNodePosition(oNode, self.xshift, self.yshift, self.zshift)
        elif(self.nodes.__len__()==1):
            oNode=self.nodes[self.nodes.keys()[0]]
            self.setOgreNodePosition(oNode, self.xshift, self.yshift, self.zshift)

        # rotate each node
        for k, oNode in self.nodes.iteritems():
            self.rotateOgreNode(oNode, q)
            pass
            
    def vectorLen(self, v):
        return sqrt(v.x()**2 + v.y()**2 + v.z()**2)
            
    def calibrateVec(self, r, v):
        if(self.vectorLen(r)!=0):
            factor = self.vectorLen(v)/self.vectorLen(r)
            rx = factor* r.x()
            ry = factor* r.y()
            rz = factor* r.z()
            return Vec(rx, ry, rz)
        else: # if original vector is zero length, the rotated one must be too, just return r
            return r
            
    def rotateOgreNode(self, on, q):
        e = on.naali_ent
        o = on.orientation
        p=e.placeable
        on.orientation = on.orientation * q
        p.Orientation = on.orientation
        pass
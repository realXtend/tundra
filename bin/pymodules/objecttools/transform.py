from __future__ import division

import rexviewer as r
import naali
rend = naali.renderer

import mathutils as mu
import math

import PythonQt

from PythonQt.QtCore import Qt

import PythonQt.QtGui
from PythonQt.QtGui import QQuaternion
from PythonQt.QtGui import QVector3D

from PythonQt.private import EC_Ruler

try:
    qapp = PythonQt.Qt.QApplication.instance()
except:
    qapp = None

def set_custom_cursor(cursor_shape):
    if qapp == None:
        return
    cursor = PythonQt.QtGui.QCursor(cursor_shape)
    current = qapp.overrideCursor()
    if current != None:
        if current.shape() != cursor_shape:
            qapp.setOverrideCursor(cursor)
    else:
        qapp.setOverrideCursor(cursor)
    
def remove_custom_cursor(cursor_shape):
    if qapp == None:
        return
    curr_cursor = qapp.overrideCursor()
    if curr_cursor != None:
        if curr_cursor.shape() == cursor_shape:
            qapp.restoreOverrideCursor()

def remove_custom_cursors():
    if qapp == None:
        return
    curr_cursor = qapp.overrideCursor()
    while curr_cursor != None:
        qapp.restoreOverrideCursor()
        curr_cursor = qapp.overrideCursor()
    
class Manipulator:
    NAME = "Manipulator"
    MANIPULATOR_MESH_NAME = "axes.mesh"
    USES_MANIPULATOR = True
    CURSOR_HOVER_SHAPE = Qt.OpenHandCursor
    CURSOR_HOLD_SHAPE = Qt.ClosedHandCursor
    
    MANIPULATORORIENTATION = QQuaternion(1, 1, 0, 0)
    MANIPULATORSCALE = QVector3D(1, 1, 1)

    MANIPULATOR_RULER_TYPE = EC_Ruler.Rotation
    
    MATERIALNAMES = None
    
    AXIS_RED = 0
    AXIS_GREEN = 1
    AXIS_BLUE = 2
    
    def __init__(self):
        #self.controller = creator
        self.manipulator = None
        self.grabbed_axis = None
        self.grabbed = False
        self.usesManipulator = self.USES_MANIPULATOR    
        self.highlightedSubMesh = None
        self.axisSubmesh = None

        self.entCenterVectors={}
        self.centerPoint = None

    def compareIds(self, id):
        if self.usesManipulator:
            if self.manipulator.Id == id:
                return True
        return False
        
    def moveTo(self, ents):
        if self.manipulator:
            pos = self.getPivotPos(ents)
            self.manipulator.placeable.Position = pos
            
    def getManipulatorPosition(self):
        if self.manipulator:
            return self.manipulator.placeable.Position
        return None
    
    def createManipulator(self):
        if self.manipulator is None and self.usesManipulator:
            # create a local temporary entity, we dont want to sync this at least for now
            # as there is no deletion code when you leave the server!
            ent = naali.createMeshEntity(self.MANIPULATOR_MESH_NAME, 606847240, ["EC_Gizmo", "EC_Ruler"], localonly = True, temporary = True)
            ent.SetName(self.NAME)
            ent.ruler.SetVisible(False)
            ent.ruler.SetType(self.MANIPULATOR_RULER_TYPE)
            ent.ruler.UpdateRuler()
            return ent

    def stopManipulating(self):
        self.grabbed_axis = None
        self.grabbed = False
        remove_custom_cursor(self.CURSOR_HOLD_SHAPE)
        try:
            self.manipulator.ruler.EndDrag()
        except:
            # TODO fix stopManipulating usage so it isn't called when manipulators aren't initialised properly yet
            pass
    
    def initVisuals(self):
        if self.manipulator is None:
            self.manipulator = self.createManipulator()
            self.hideManipulator()
        
    def showManipulator(self, ents):
        if self.usesManipulator and len(ents)>0:
            self.moveTo(ents)
            self.setManipulatorScale(ents)
            try:
                ruler = self.manipulator.ruler
            except:
                ruler = self.manipulator.GetOrCreateComponentRaw("EC_Ruler")
            ruler.SetType(self.MANIPULATOR_RULER_TYPE)
            ruler.SetVisible(True)
            #r.logInfo("showing ruler showManipulator")
            ruler.UpdateRuler()
            if False: # self.controller.useLocalTransform:
                # first according object, then manipulator orientation - otherwise they go wrong order
                self.manipulator.placeable.Orientation = ents[0].placeable.Orientation * self.MANIPULATORORIENTATION
            else:
                self.manipulator.placeable.Orientation = self.MANIPULATORORIENTATION
            
            
    def getPivotPos(self, ents):        
        '''Median position used as pivot point'''
        xs = [e.placeable.Position.x() for e in ents]
        ys = [e.placeable.Position.y() for e in ents]
        zs = [e.placeable.Position.z() for e in ents]
                
        minpos = QVector3D(min(xs), min(ys), min(zs))
        maxpos = QVector3D(max(xs), max(ys), max(zs))
        median = (minpos + maxpos) / 2 
        
        return median
        
    def hideManipulator(self):
        #r.logInfo("hiding manipulator")
        if self.usesManipulator:
            try: #XXX! without this try-except, if something is selected, the viewer will crash on exit
                #print "Hiding arrows!"
                if self.manipulator is not None:
                    self.manipulator.placeable.Scale = QVector3D(0.0, 0.0, 0.0) #ugly hack
                    self.manipulator.placeable.Position = QVector3D(0.0, 0.0, 0.0)#another ugly hack
                    self.manipulator.ruler.SetVisible(False)
                    #r.logInfo("hiding ruler hideManipulator")
                    self.manipulator.ruler.UpdateRuler()
                self.grabbed_axis = None
                self.grabbed = False
                remove_custom_cursors()
            except RuntimeError, e:
                r.logDebug("hideManipulator failed")
            except AttributeError, e:
                # this will happend on_exit() -> hideManipulator
                # the manipulator entitys components has been destroyed
                pass
                
    
    def initManipulation(self, ent, results, ents):
        if self.usesManipulator:
            if ent is None:
                return

            if ent.Id == self.manipulator.Id:
                submeshid = results[-3]
                self.axisSubmesh = submeshid
                u = results[-2]
                v = results[-1]
                self.grabbed = True
                if submeshid in self.BLUEARROW:
                    #~ print "arrow is blue"
                    self.grabbed_axis = self.AXIS_BLUE
                elif submeshid in self.GREENARROW:
                    #~ print "arrow is green"
                    self.grabbed_axis = self.AXIS_GREEN
                elif submeshid in self.REDARROW:
                    #~ print "arrow is red"
                    self.grabbed_axis = self.AXIS_RED
                else:
                    #~ print "arrow got screwed..."
                    self.grabbed_axis = None
                    self.grabbed = False
                    
                if self.grabbed_axis != None:
                    self.manipulator.ruler.SetAxis(self.grabbed_axis)
                    self.manipulator.ruler.SetVisible(True)
                    #r.logInfo("show ruler initManipulation")
                    self.manipulator.ruler.UpdateRuler()
                    if ents[0]:
                        placeable = ents[0].placeable
                        self.manipulator.ruler.StartDrag(placeable.Position, placeable.Orientation, placeable.Scale)
                    set_custom_cursor(self.CURSOR_HOLD_SHAPE)
                else:
                    remove_custom_cursor(self.CURSOR_HOLD_SHAPE)
                    self.manipulator.ruler.SetVisible(False)
                    #r.logInfo("hide ruler initManipulation")
                    self.manipulator.ruler.UpdateRuler()

    def setManipulatorScale(self, ents):
        if ents is None or len(ents) == 0: 
                return

        campos = naali.getCamera().placeable.Position
        ent = ents[-1]
        entpos = ent.placeable.Position
        length = (campos-entpos).length()
            
        v = self.MANIPULATORSCALE
        factor = length*.1
        newv = QVector3D(v) * factor
        try:
            self.manipulator.placeable.Scale = newv
        except AttributeError:
            pass
                    
    def manipulate(self, ents, movedx, movedy):
        if ents is not None:
            fov = naali.getCamera().camera.GetVerticalFov()
            width, height = rend.GetWindowWidth(), rend.GetWindowHeight()
            campos = naali.getCamera().placeable.Position
            ent = ents[-1]
            entpos = ent.placeable.Position
            length = (campos-entpos).length()
                
            worldwidth = (math.tan(fov/2)*length) * 2
            worldheight = (height*worldwidth) / width

            ## used in freemoving to get the size of movement right
            # factor for move size
            movefactor = width / height
            amountx = (worldwidth * movedx) * movefactor
            amounty = (worldheight * movedy) * movefactor

            self.setManipulatorScale(ents)

            rightvec = mu.get_right(naali.getCamera())
            upvec = mu.get_up(naali.getCamera())

            rightvec *= amountx
            upvec *= amounty
            changevec = rightvec - upvec

            # group rotation
            if self.NAME=="RotationManipulator" and len(ents)>1 and self.grabbed_axis == self.AXIS_BLUE:
                self.setCenterPointAndCenterVectors(ents)
                self._manipulate2(ents, amountx, amounty, changevec, self.entCenterVectors, self.centerPoint)
            else:            
                for ent in ents:
                    self._manipulate(ent, amountx, amounty, changevec)
                    #self.controller.soundRuler(ent)

                if not self.manipulator is None:
                    if len(ents) > 0 and self.NAME!="FreeMoveManipulator":
                        placeable = ents[0].placeable
                        #self.manipulator.ruler.DoDrag(placeable.Position, placeable.Orientation, placeable.Scale)

                #self.manipulator.ruler.UpdateRuler()
                
            if self.usesManipulator:
                self.moveTo(ents)
                
    def hasParent(self, ent):
        qprim = ent.prim
        if qprim is not None and qprim.ParentId == 0:
            return False
        return True
    
    def highlight(self, raycast_results):
        if self.usesManipulator and self.MATERIALNAMES is not None:
            
            if self.highlightedSubMesh is not None:
                self.resethighlight()
            
            submeshid = raycast_results[-3]
            if submeshid >= 0:
                name =  self.MATERIALNAMES[submeshid]
                if name is not None:
                    name += str("_hi")
                    self.manipulator.mesh.SetMaterial(submeshid, name)
                    self.highlightedSubMesh = submeshid
                    set_custom_cursor(self.CURSOR_HOVER_SHAPE)

    def resethighlight(self):
        if self.usesManipulator and self.highlightedSubMesh is not None:
            name = self.MATERIALNAMES[self.highlightedSubMesh]
            if name is not None:
                self.manipulator.mesh.SetMaterial(self.highlightedSubMesh, name)
            self.highlightedSubMesh = None
            remove_custom_cursors()
            
    def setCenterPointAndCenterVectors(self, ents):
        self.centerPoint = self.getPivotPos(ents)
        for ent in ents:
            pos = ent.placeable.Position
            diff = self.vectorDifference(pos, self.centerPoint)
            self.entCenterVectors[ent]=diff 
        # print "center vectors"
        # for e, vec in self.entCenterVectors.iteritems():
            # print vec
        
    def vectorDifference(self, v1, v2):
        x = v1.x()-v2.x()
        y = v1.y()-v2.y()
        z = v1.z()-v2.z()
        return QVector3D(x, y, z)

    def calibrateVec(self, r, v):
        if(self.vectorLen(r)!=0):
            factor = self.vectorLen(v)/self.vectorLen(r)
            rx = factor* r.x()
            ry = factor* r.y()
            rz = factor* r.z()
            return QVector3D(rx, ry, rz)
        else: # if original vector is zero length, the rotated one must be too, just return r
            return r

    def vectorLen(self, v):
        return math.sqrt(v.x()**2 + v.y()**2 + v.z()**2)

    def vectorAdd(self, v1, v2):
        x = v1.x()+v2.x()
        y = v1.y()+v2.y()
        z = v1.z()+v2.z()
        return QVector3D(x, y, z)
        
class MoveManipulator(Manipulator):
    NAME = "MoveManipulator"
    MANIPULATOR_MESH_NAME = "axis1.mesh"
    MANIPULATOR_RULER_TYPE = EC_Ruler.Translation
    
    GREENARROW = [0]
    REDARROW = [1]
    BLUEARROW = [2]

    AXIS_GREEN = 0
    AXIS_RED = 1
    AXIS_BLUE = 2
    
    MATERIALNAMES = {
        0: "axis_green",
        1: "axis_red",
        2: "axis_blue"
    }

    def _manipulate(self, ent, amountx, amounty, changevec):
        if self.grabbed:
            if False: #self.controller.useLocalTransform:
                if self.grabbed_axis == self.AXIS_RED:
                    ent.network.Position = ent.placeable.translate(0, -changevec.x())
                elif self.grabbed_axis == self.AXIS_GREEN:
                    ent.network.Position = ent.placeable.translate(1, -changevec.y())
                elif self.grabbed_axis == self.AXIS_BLUE:
                    ent.network.Position = ent.placeable.translate(2, changevec.z())
            else:
                if self.grabbed_axis == self.AXIS_BLUE:
                    changevec.setX(0)
                    changevec.setY(0)
                elif self.grabbed_axis == self.AXIS_RED:
                    changevec.setZ(0)
                    changevec.setY(0)
                elif self.grabbed_axis == self.AXIS_GREEN:
                    changevec.setZ(0)
                    changevec.setX(0)
                ent.placeable.Position += changevec
#                ent.network.Position += changevec

class ScaleManipulator(Manipulator):
    NAME = "ScaleManipulator"
    MANIPULATOR_MESH_NAME = "scale1.mesh"
    MANIPULATOR_RULER_TYPE = EC_Ruler.Scale

    MATERIALNAMES = {
        0: "axis_green",
        1: "axis_red",
        2: "axis_blue"
    }

    AXIS_GREEN = 0
    AXIS_RED = 1
    AXIS_BLUE = 2
    
    GREENARROW = [0]
    REDARROW = [1]
    BLUEARROW = [2]
    
    def _manipulate(self, ent, amountx, amounty, changevec):
        if self.grabbed:
            if self.grabbed_axis == self.AXIS_BLUE:
                changevec.setX(0)
                changevec.setY(0)
            elif self.grabbed_axis == self.AXIS_RED:
                changevec.setZ(0)
                changevec.setY(0)
            elif self.grabbed_axis == self.AXIS_GREEN:
                changevec.setX(0)
                changevec.setZ(0)
            
            ent.placeable.Scale += changevec
            
class FreeMoveManipulator(Manipulator):
    NAME = "FreeMoveManipulator"
    USES_MANIPULATOR = False
    
    """ Using Qt's QVector3D. This has some lag issues or rather annoying stutterings """
    def _manipulate(self, ent, amountx, amounty, changevec):
        ent.placeable.Position += changevec
        #ent.network.Position += changevec
        
class RotationManipulator(Manipulator):
    NAME = "RotationManipulator"
    MANIPULATOR_MESH_NAME = "rotate1.mesh"
    MANIPULATOR_RULER_TYPE = EC_Ruler.Rotation
    
    MATERIALNAMES = {
        0: "axis_green",
        1: "axis_red",
        2: "axis_blue"
    }

    AXIS_GREEN = 0
    AXIS_RED = 1
    AXIS_BLUE = 2
    
    GREENARROW = [0] # we do blue_axis actions
    REDARROW = [1]
    BLUEARROW = [2] # we do green_axis actions
    
    """ Using Qt's QQuaternion. This bit has some annoying stuttering aswell... """
    def _manipulate(self, ent, amountx, amounty, changevec):
        if self.grabbed and self.grabbed_axis is not None:
            local = False #self.controller.useLocalTransform
            mov = changevec.length() * 15
            ort = ent.placeable.Orientation

            if amountx < 0 and amounty < 0:
                dir = -1
            elif amountx < 0 and amounty >= 0:
                dir = 1
                if not local and self.grabbed_axis == self.AXIS_BLUE:
                    dir *= -1
            elif amountx >= 0 and amounty < 0:
                dir = -1
            elif amountx >= 0 and amounty >= 0:
                dir = 1

            mov *= dir

            if local:
                if self.grabbed_axis == self.AXIS_RED:
                    axis = QVector3D(1, 0, 0)
                elif self.grabbed_axis == self.AXIS_GREEN:
                    axis = QVector3D(0, 1, 0)
                elif self.grabbed_axis == self.AXIS_BLUE:
                    axis = QVector3D(0, 0, 1)

                ort = ort * QQuaternion.fromAxisAndAngle(axis, mov)
            else:
                euler = mu.quat_to_euler(ort)
                if self.grabbed_axis == self.AXIS_RED: #rotate around x-axis
                    euler[0] -= math.radians(mov)
                elif self.grabbed_axis == self.AXIS_GREEN: #rotate around y-axis
                    euler[1] += math.radians(mov)
                elif self.grabbed_axis == self.AXIS_BLUE: #rotate around z-axis
                    euler[2] += math.radians(mov)

                ort = mu.euler_to_quat(euler)

            ent.placeable.Orientation = ort
            #ent.network.Orientation = ort
    
    """ Rotate locations around center point """
    def _manipulate2(self, ents, amountx, amounty, changevec, centervecs, centerpoint):
        # calculate quaternion for rotation
        # get axis
        # print "amountx %s"%amountx
        # print "amounty %s"%amounty
        # print "changevec %s"%changevec
        
        if self.grabbed and self.grabbed_axis is not None:
            local = False # self.controller.useLocalTransform
            mov = changevec.length() * 30

            axis = None
            #angle = 90 # just do 90 degrees rotations
            #angle = 15 # just do 15 degrees rotations
            angle = 5 # just do 5 degrees rotations
            
            if amountx < 0 and amounty < 0:
                dir = -1
            elif amountx < 0 and amounty >= 0:
                dir = 1
                if self.grabbed_axis == self.AXIS_BLUE:
                    dir *= -1
            elif amountx >= 0 and amounty < 0:
                dir = -1
            elif amountx >= 0 and amounty >= 0:
                dir = 1

            mov *= dir
            angle *= dir
            q = None
            
            euler = None
            if self.grabbed_axis == self.AXIS_RED: #rotate around x-axis
                axis = QVector3D(1,0,0)
                # disable this for now
                # return 
            elif self.grabbed_axis == self.AXIS_GREEN: #rotate around y-axis
                axis = QVector3D(0,1,0)
                # disable this for now
                # return
            elif self.grabbed_axis == self.AXIS_BLUE: #rotate around z-axis
                axis = QVector3D(0,0,1)

            q = QQuaternion.fromAxisAndAngle(axis, angle)
            q.normalize()

            self._rotateEntsWithQuaternion(q, ents, amountx, amounty, changevec, centervecs, centerpoint)
            self._rotateEachEntWithQuaternion(q, ents, angle)
        pass
        
    def _rotateEntsWithQuaternion(self, q, ents, amountx, amounty, changevec, centervecs, centerpoint):
        for ent, qvec in centervecs.iteritems():
            # rotate center vectors and calculate new points to ents
            # print "qvec %s"%str(qvec)
            crot=q.rotatedVector(qvec) # rotated center vector
            # print "crot %s"%str(crot)
            calibVec = self.calibrateVec(crot, qvec) # just incase
            centervecs[ent]=calibVec # store new rotated vector
            
        for ent, newVec in centervecs.iteritems():
            # print "centerpoint %s"%centerpoint
            # print "newVec %s"%newVec
            newPos = self.vectorAdd(centerpoint, newVec)
            if hasattr(ent, "placeable"):
                ent.placeable.Position = newPos
                #ent.network.Position = newPos
            else:
                print "entity missing placeable"
                # print type(ent)
        pass

    """ This part still has some issues, z-group rotation now working perfectly,
        x and y group rotations still go bonkers, y-rotation stops when limit -1.0 or 1.0 rads
        is reached, and x-rotation is totally wrong
        also this method functionality probably overlaps with above single object specific rotation, 
        anyway keeping it separate untill multirotate works correctly """
    def _rotateEachEntWithQuaternion(self, q, ents, angle):
        for ent in ents:
            ort = ent.placeable.Orientation
            euler = mu.quat_to_euler(ort)
            if self.grabbed_axis == self.AXIS_RED: #rotate around x-axis
                #print euler[0] 
                #print math.radians(angle)
                euler[0] += math.radians(angle)
            elif self.grabbed_axis == self.AXIS_GREEN: #rotate around y-axis
                #print euler[1] 
                #print math.radians(angle)
                euler[1] += math.radians(angle)
            elif self.grabbed_axis == self.AXIS_BLUE: #rotate around z-axis
                #print euler[2] 
                #print math.radians(angle)
                euler[2] += math.radians(angle)
            ort = mu.euler_to_quat(euler)
            ent.placeable.Orientation = ort
            #ent.network.Orientation = ort
        pass

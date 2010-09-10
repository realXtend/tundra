from __future__ import division

import rexviewer as r
import naali
rend = naali.renderer

import math

import PythonQt
import PythonQt.QtGui

from PythonQt.QtCore import Qt
from PythonQt.QtGui import QQuaternion as Quat
from PythonQt.QtGui import QVector3D as Vec

try:
    qapp = PythonQt.Qt.QApplication.instance()
except:
    qapp = None

# according Quaternion*Vector from irrlicht (see Core\Quaternion.h)
def quat_mult_vec(quat, v):
    qvec = quat.vector()
    uv = Vec.crossProduct(qvec, v)
    uuv = Vec.crossProduct(qvec, uv)
    uv = uv * 2.0 * quat.scalar()
    uuv = uuv * 2.0

    return v + uv + uuv

# QQuaternion to euler [x,y,z]
def quat_to_euler(quat):
    euler = [0, 0, 0]
    sqw = quat.scalar() * quat.scalar()
    sqx = quat.x() * quat.x()
    sqy = quat.y() * quat.y()
    sqz = quat.z() * quat.z()
    
    euler[2] = math.atan2(2.0 * (quat.x()*quat.y() +quat.z()*quat.scalar()),(sqx - sqy - sqz + sqw))
    euler[0] = math.atan2(2.0 * (quat.y()*quat.z() +quat.x()*quat.scalar()),(-sqx - sqy + sqz + sqw))
    yval = math.asin(-2.0 * (quat.x()*quat.z() - quat.y()*quat.scalar()))
    if yval < -1.0:
        yval = -1.0
    elif yval > 1.0:
        yval = 1.0
    euler[1] = yval
    
    return euler

# euler [x,y,z] to QQuaternion
def euler_to_quat(euler):
    ang = euler[0] * 0.5
    sr = math.sin(ang)
    cr = math.cos(ang)

    ang = euler[1] * 0.5
    sp = math.sin(ang)
    cp = math.cos(ang)

    ang = euler[2] * 0.5
    sy = math.sin(ang)
    cy = math.cos(ang)

    cpcy = cp * cy
    spcy = sp * cy
    cpsy = cp * sy
    spsy = sp * sy

    quat = Quat(cr*cpcy + sr*spsy, sr * cpcy - cr*spsy, cr*spcy + sr * cpsy, cr * cpsy - sr * spcy)
    quat.normalize()
    return quat

# replacement for r.GetCameraUp()
def get_up(entity):
    v = Vec(0.0, 1.0, 0.0)
    return quat_mult_vec(entity.placeable.Orientation, v)
    
# replacement for r.GetCameraRight()
def get_right(entity):
    v = Vec(1.0, 0.0, 0.0)
    return quat_mult_vec(entity.placeable.Orientation, v)

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
    
    MANIPULATORORIENTATION = Quat(1, 0, 0, 0)
    MANIPULATORSCALE = Vec(1, 1, 1)
    
    MATERIALNAMES = None
    
    AXIS_RED = 0
    AXIS_GREEN = 1
    AXIS_BLUE = 2

    # some handy shortcut rotations for quats
    ninty_around_x = Quat(math.sqrt(0.5), math.sqrt(0.5), 0, 0)
    ninty_around_y = Quat(math.sqrt(0.5), 0, math.sqrt(0.5), 0)
    ninty_around_z = Quat(math.sqrt(0.5), 0, 0, math.sqrt(0.5))
    
    def __init__(self, creator):
        self.controller = creator
        self.manipulator = None
        self.grabbed_axis = None
        self.grabbed = False
        self.usesManipulator = self.USES_MANIPULATOR    
        self.manipulator = None
        self.highlightedSubMesh = None
        self.axisSubmesh = None

    def compareIds(self, id):
        if self.usesManipulator:
            if self.manipulator.id == id:
                return True
        return False
        
    def moveTo(self, ents):
        if self.manipulator:
            pos = self.getPivotPos(ents)
            #print "Showing at: ", pos
            self.manipulator.placeable.Position = pos
            
    def getManipulatorPosition(self):
        if self.manipulator:
            return self.manipulator.placeable.Position
        return None
    
    def createManipulator(self):
        if self.manipulator is None and self.usesManipulator:
            ent = r.createEntity(self.MANIPULATOR_MESH_NAME, 606847240) 
            return ent 

    def stopManipulating(self):
        self.grabbed_axis = None
        self.grabbed = False
        remove_custom_cursor(self.CURSOR_HOLD_SHAPE)
    
    def initVisuals(self):
        #r.logInfo("initVisuals in manipulator " + str(self.NAME))
        if self.manipulator is None:
            self.manipulator = self.createManipulator()
            self.hideManipulator()
        
    def showManipulator(self, ents):
        #print "Showing arrows!"
        if self.usesManipulator and len(ents)>0:
            self.moveTo(ents)
            self.manipulator.placeable.Scale = self.MANIPULATORSCALE
            if self.controller.useLocalTransform:
                # first according object, then manipulator orientation - otherwise they go wrong order
                self.manipulator.placeable.Orientation = ents[0].placeable.Orientation * self.MANIPULATORORIENTATION
            else:
                self.manipulator.placeable.Orientation = self.MANIPULATORORIENTATION

        self.setManipulatorScale(ents)
            
    def getPivotPos(self, ents):        
        '''Median position used as pivot point'''
        xs = [e.placeable.Position.x() for e in ents]
        ys = [e.placeable.Position.y() for e in ents]
        zs = [e.placeable.Position.z() for e in ents]
                
        minpos = Vec(min(xs), min(ys), min(zs))
        maxpos = Vec(max(xs), max(ys), max(zs))
        median = (minpos + maxpos) / 2 
        
        return median
        
    def hideManipulator(self):
        #r.logInfo("hiding manipulator")
        if self.usesManipulator:
            try: #XXX! without this try-except, if something is selected, the viewer will crash on exit
                #print "Hiding arrows!"
                if self.manipulator is not None:
                    self.manipulator.placeable.Scale = Vec(0.0, 0.0, 0.0) #ugly hack
                    self.manipulator.placeable.Position = Vec(0.0, 0.0, 0.0)#another ugly hack
                
                self.grabbed_axis = None
                self.grabbed = False
                remove_custom_cursors()
                
            except RuntimeError, e:
                r.logDebug("hideManipulator failed")
    
    def initManipulation(self, ent, results):
        if self.usesManipulator:
            
            if ent is None:
                return

            if ent.id == self.manipulator.id:
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
                    set_custom_cursor(self.CURSOR_HOLD_SHAPE)
                else:
                    remove_custom_cursor(self.CURSOR_HOLD_SHAPE)

    def setManipulatorScale(self, ents):
        if ents is None or len(ents) == 0: 
                return

        campos = naali.getCamera().placeable.Position
        ent = ents[-1]
        entpos = ent.placeable.Position
        length = (campos-entpos).length()
            
        v = self.MANIPULATORSCALE
        factor = length*.1
        newv = Vec(v) * factor
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
                
            #used in freemoving to get the size of movement right
            amountx = (worldwidth * movedx)
            amounty = (worldheight * movedy)

            self.setManipulatorScale(ents)

            rightvec = get_right(naali.getCamera())
            upvec = get_up(naali.getCamera())

            rightvec *= amountx
            upvec *= amounty
            changevec = rightvec - upvec
            
            for ent in ents:
                self._manipulate(ent, amountx, amounty, changevec)
                self.controller.soundRuler(ent)
                
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
        
class MoveManipulator(Manipulator):
    NAME = "MoveManipulator"
    MANIPULATOR_MESH_NAME = "axis1.mesh"
    
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
#            rightvec = get_right(naali.getCamera())
#            upvec = get_up(naali.getCamera())
#            rightvec *= amountx
#            upvec *= amounty
#            changevec = rightvec - upvec

            if self.controller.useLocalTransform:
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
                ent.network.Position += changevec

class ScaleManipulator(Manipulator):
    NAME = "ScaleManipulator"
    MANIPULATOR_MESH_NAME = "scale1.mesh"

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
            qprim = ent.prim
            if qprim is not None:
                children = qprim.GetChildren()
                for child_id in children: #XXX this might not be the wanted behaviour with linksets! .. when just scaling the rootpart.
                    child = r.getEntity(int(child_id))
                    child.placeable.Scale += changevec
            
class FreeMoveManipulator(Manipulator):
    NAME = "FreeMoveManipulator"
    USES_MANIPULATOR = False
    
    """ Using Qt's QVector3D. This has some lag issues or rather annoying stutterings """
    def _manipulate(self, ent, amountx, amounty, changevec):
#        rightvec = get_right(naali.getCamera())
#        upvec = get_up(naali.getCamera())
#
#        rightvec *= amountx
#        upvec *= amounty
#        changevec = rightvec - upvec

        ent.placeable.Position += changevec
        ent.network.Position += changevec
        
class RotationManipulator(Manipulator):
    NAME = "RotationManipulator"
    MANIPULATOR_MESH_NAME = "rotate1.mesh"
    
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
            local = self.controller.useLocalTransform
            mov = changevec.length() * 30
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
                    axis = Vec(1, 0, 0)
                elif self.grabbed_axis == self.AXIS_GREEN:
                    axis = Vec(0, 1, 0)
                elif self.grabbed_axis == self.AXIS_BLUE:
                    axis = Vec(0, 0, 1)

                ort = ort * Quat.fromAxisAndAngle(axis, mov)
            else:
                euler = quat_to_euler(ort)

                if self.grabbed_axis == self.AXIS_RED: #rotate around x-axis
                    euler[0] -= math.radians(mov)
                elif self.grabbed_axis == self.AXIS_GREEN: #rotate around y-axis
                    euler[1] += math.radians(mov)
                elif self.grabbed_axis == self.AXIS_BLUE: #rotate around z-axis
                    euler[2] += math.radians(mov)

                ort = euler_to_quat(euler)

            ent.placeable.Orientation = ort
            ent.network.Orientation = ort


import rexviewer as r
import PythonQt.QtGui
from PythonQt.QtGui import QQuaternion as Quat
from PythonQt.QtGui import QVector3D as Vec
from vector3 import Vector3 #for view based editing calcs now that Vector3 not exposed from internals
from conversions import quat_to_euler, euler_to_quat #for euler - quat -euler conversions
import math

class Manipulator:
    NAME = "Manipulator"
    MANIPULATOR_MESH_NAME = "axes.mesh"
    USES_MANIPULATOR = True
    
    MANIPULATORORIENTATION = Quat(1, 0, 0, 0)
    MANIPULATORSCALE = Vec(0.2, 0.2, 0.2)
    
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
    
    def initVisuals(self):
        #r.logInfo("initVisuals in manipulator " + str(self.NAME))
        if self.manipulator is None:
            self.manipulator = self.createManipulator()
            self.hideManipulator()
        
    def showManipulator(self, ents):
        #print "Showing arrows!"
        if self.usesManipulator:
            self.moveTo(ents)
            self.manipulator.placeable.Scale = self.MANIPULATORSCALE
            if self.controller.useLocalTransform:
                # first according object, then manipulator orientation - otherwise they go wrong order
                self.manipulator.placeable.Orientation = ents[0].placeable.Orientation * self.MANIPULATORORIENTATION
            else:
                self.manipulator.placeable.Orientation = self.MANIPULATORORIENTATION
            
    def getPivotPos(self, ents):        
        xs = [e.placeable.Position.x() for e in ents]
        ys = [e.placeable.Position.y() for e in ents]
        zs = [e.placeable.Position.z() for e in ents] 
                
        minpos = Vec(min(xs), min(ys), min(zs))
        maxpos = Vec(max(xs), max(ys), max(zs))
        #median = (minpos + maxpos) / 2
        #there is some type prob with pythonqt and operator overloads, so this workaround is needed:
        median = (minpos + maxpos) / 2 
        #print "Min:", minpos
        #print "Max:", minpos
        #print "Median:", median
        
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
                
            except RuntimeError, e:
                r.logDebug("hideManipulator failed")
    
    def initManipulation(self, ent, results):
        if self.usesManipulator:
            
            if ent is None:
                return
                
            if ent.id == self.manipulator.id:
                    submeshid = results[-3]
                    u = results[-2]
                    v = results[-1]
                    #print "ARROW and UV", u, v
                    #print submeshid
                    self.grabbed = True
                    if submeshid in self.BLUEARROW or (u != 0.0 and u < 0.421875):
                        #~ print "arrow is blue"
                        self.grabbed_axis = self.AXIS_BLUE
                    elif submeshid in self.GREENARROW or (u != 0.0 and u < 0.70703125):
                        #~ print "arrow is green"
                        self.grabbed_axis = self.AXIS_GREEN
                    elif submeshid in self.REDARROW or (u != 0.0 and u <= 1.0):
                        #~ print "arrow is red"
                        self.grabbed_axis = self.AXIS_RED
                    else:
                        #~ print "arrow got screwed..."
                        self.grabbed_axis = None
                        self.grabbed = False
                    
    def manipulate(self, ents, movedx, movedy):
        if ents is not None:
            lengthx = 0
            lengthy = 0
            fov = r.getCameraFOV()
            width, height = r.getScreenSize()
            campos = Vector3(r.getCameraPosition())
            ent = ents[-1]
            qpos = ent.placeable.Position
            entpos = Vector3(qpos.x(), qpos.y(), qpos.z())
            length = (campos-entpos).length
                
            worldwidth = (math.tan(fov/2)*length) * 2
            worldheight = (height*worldwidth) / width
                
            #used in freemoving to get the size of movement right
            amountx = (worldwidth * movedx)
            amounty = (worldheight * movedy)
            
            if self.usesManipulator and self.grabbed_axis is not None:
                rightvec = Vector3(r.getCameraRight())
                upvec = Vector3(r.getCameraUp())
                temp = [0,0,0]
                temp[self.grabbed_axis] = 1
                axis_vec = Vector3(temp)
                #print amountx, amounty
                mousey_on_arrow_projection = upvec.dot(axis_vec) * axis_vec
                lengthy = mousey_on_arrow_projection.length * amounty
                mousex_on_arrow_projection = rightvec.dot(axis_vec) * axis_vec
                lengthx = mousex_on_arrow_projection.length * amountx
            
            for ent in ents:
                self._manipulate(ent, amountx, amounty, lengthx, lengthy)
                
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
            if submeshid > 0:
                name =  self.MATERIALNAMES[submeshid]
                if name is not None:
                    name += str("_hi")
                    self.manipulator.mesh.SetMaterial(submeshid, name)
                    self.highlightedSubMesh = submeshid

    def resethighlight(self):
        if self.usesManipulator and self.highlightedSubMesh is not None:
            name = self.MATERIALNAMES[self.highlightedSubMesh]
            if name is not None:
                self.manipulator.mesh.SetMaterial(self.highlightedSubMesh, name)
            self.highlightedSubMesh = None
        
class MoveManipulator(Manipulator):
    NAME = "MoveManipulator"
    MANIPULATOR_MESH_NAME = "axis1.mesh"
    
    MANIPULATORSCALE = Vec(0.15, 0.15, 0.15)
    # multiply the two orientations, so we get the proper end orientation for the widget
    MANIPULATORORIENTATION = Manipulator.ninty_around_x * Manipulator.ninty_around_y
    
    BLUEARROW = [1,2]
    REDARROW = [5,6]
    GREENARROW = [3,4]

    AXIS_RED = 1
    AXIS_GREEN = 0
    AXIS_BLUE = 2
    
    
    MATERIALNAMES = {
        0: "axis_black",  #shodows?
        1: "axis_blue", 
        2: None, #"axis_blue", 
        3: "axis_green",
        4: None, #"axis_green",
        5: "axis_red", 
        6: None, #"axis_red"
    }

    def _manipulate(self, ent, amountx, amounty, lengthx, lengthy):
        if self.grabbed:
            rightvec = Vector3(r.getCameraRight())
            upvec = Vector3(r.getCameraUp())
            qpos = ent.placeable.Position
            if self.grabbed_axis == self.AXIS_BLUE:
                mov = lengthy
                qpos.setZ(qpos.z()-mov)
            else:
                mov = lengthx 
                div = abs(rightvec[self.grabbed_axis])
                if div == 0:
                    div = 0.00001 #not the best of ideas but...
                mov *= rightvec[self.grabbed_axis]/div
                if self.grabbed_axis == self.AXIS_GREEN:
                    qpos.setY(qpos.y()-mov)
                else:
                    qpos.setX(qpos.x()-mov)

            ent.placeable.Position = qpos
            ent.network.Position = qpos

class ScaleManipulator(Manipulator):
    NAME = "ScaleManipulator"
    #MANIPULATOR_MESH_NAME = "axes.mesh"
    
    BLUEARROW = [3]#not used
    REDARROW = [1]
    GREENARROW = [2]
    
    def _manipulate(self, ent, amountx, amounty, lengthx, lengthy):
        if self.grabbed:
            qscale = ent.placeable.Scale
            scale = list((qscale.x(), qscale.y(), qscale.z()))
            rightvec = Vector3(r.getCameraRight())
            upvec = Vector3(r.getCameraUp())
            
            if self.grabbed_axis == self.AXIS_BLUE:
                mov = lengthy
                scale[self.grabbed_axis] -= mov
            else:
                mov = lengthx
                div = abs(rightvec[self.grabbed_axis])
                if div == 0:
                    div = 0.01 #not the best of ideas but...
                mov *= rightvec[self.grabbed_axis]/div
                scale[self.grabbed_axis] += mov
            
            newscale = Vec(scale[0], scale[1], scale[2])
            ent.placeable.Scale = newscale
            qprim = ent.prim
            if qprim is not None:
                children = qprim.GetChildren()
                for child_id in children: #XXX this might not be the wanted behaviour with linksets! .. when just scaling the rootpart.
                    child = r.getEntity(int(child_id))
                    child.placeable.Scale = newscale
            
class FreeMoveManipulator(Manipulator):
    NAME = "FreeMoveManipulator"
    USES_MANIPULATOR = False
    
    """ Using Qt's QVector3D. This has some lag issues or rather annoying stutterings """
    def _manipulate(self, ent, amountx, amounty, lengthx, lengthy):
        rightvec = Vector3(r.getCameraRight())
        upvec = Vector3(r.getCameraUp())
        changevec = (amountx * rightvec) - (amounty * upvec)
        qpos = ent.placeable.Position
        entpos = Vector3(qpos.x(), qpos.y(), qpos.z())
        newpos = entpos + changevec
        newpos = Vec(newpos.x, newpos.y, newpos.z)
        ent.placeable.Position = newpos
        ent.network.Position = newpos
        
    #~ def _manipulate(self, ent, amountx, amounty, lengthx, lengthy):
        #~ rightvec = Vector3(r.getCameraRight())
        #~ upvec = Vector3(r.getCameraUp())
        #~ changevec = (amountx * rightvec) - (amounty * upvec)
        #~ entpos = Vector3(ent.pos)
        #~ newpos = entpos + changevec
        #~ ent.pos = newpos.x, newpos.y, newpos.z    
        
class RotationManipulator(Manipulator):
    NAME = "RotationManipulator"
    MANIPULATOR_MESH_NAME = "rotate1.mesh"
    
    MANIPULATORORIENTATION = Manipulator.ninty_around_x
    
    MATERIALNAMES = {
        0: "asd",  #shadows?
        1: "resed", 
        2: "resed2", 
        3: "resed3"
    }
    
    BLUEARROW = [3]
    REDARROW = [1]
    GREENARROW = [2]
    
    """ Using Qt's QQuaternion. This bit has some annoying stuttering aswell... """
    def _manipulate(self, ent, amountx, amounty, lengthx, lengthy):
        if self.grabbed and self.grabbed_axis is not None:
            rightvec = Vector3(r.getCameraRight())
            upvec = Vector3(r.getCameraUp())
            
            ort = ent.placeable.Orientation
            euler = [0, 0, 0]
            
            if self.controller.useLocalTransform:
                if self.grabbed_axis == self.AXIS_RED:
                    mov = amounty * 30
                    axis = Vec(1, 0, 0)
                elif self.grabbed_axis == self.AXIS_GREEN:
                    mov = amountx * 30 
                    axis = Vec(0, 1, 0)
                elif self.grabbed_axis == self.AXIS_BLUE:
                    mov = amountx * 30
                    axis = Vec(0, 0, 1)
                dir = 1
                math.copysign(dir, mov)

                delta = Quat.fromAxisAndAngle(axis, dir)
                ort *= delta
            else:
                if self.grabbed_axis == self.AXIS_GREEN: #rotate around y-axis
                    # print "green axis", self.grabbed_axis,
                    mov = amountx * 30 
                    euler[1] += mov
                elif self.grabbed_axis == self.AXIS_BLUE: #rotate around z-axis
                    # print "blue axis", self.grabbed_axis,
                    mov = amountx * 30
                    euler[2] += mov
                elif self.grabbed_axis == self.AXIS_RED: #rotate around x-axis
                    # print "red axis", self.grabbed_axis,
                    mov = amounty * 30
                    euler[0] -= mov
                rotationQuat = euler_to_quat(euler)
                # TODO: figure out the shifted members
                ort *= Quat(rotationQuat[3], rotationQuat[1], rotationQuat[2], rotationQuat[0])
            
            ent.placeable.Orientation = ort
            ent.network.Orientation = ort


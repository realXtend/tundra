import rexviewer as r
from vector3 import Vector3 #for view based editing calcs now that Vector3 not exposed from internals
from conversions import quat_to_euler, euler_to_quat #for euler - quat -euler conversions
import math

AXIS_RED = 0
AXIS_GREEN = 1
AXIS_BLUE = 2
    
class Manipulator:
    NAME = "Manipulator"
    MANIPULATOR_MESH_NAME = "axes.mesh"
    USES_MANIPULATOR = True
    
    MANIPULATORORIENTATION = 0, 0, 0, 1
    
    MATERIALNAMES = None
    
    def __init__(self, creator):
        self.controller = creator;
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
            self.manipulator.pos = pos
            #self.manipulator.pos = pos[0], pos[1], pos[2]
            
    def getManipulatorPosition(self):
        if self.manipulator:
            return self.manipulator.pos
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
            self.manipulator.scale = 0.2, 0.2, 0.2
            self.manipulator.orientation = self.MANIPULATORORIENTATION
    
    def getPivotPos(self, ents):
        positions = []
        
        for ent in ents:
            #print ent.id, ent.pos
            pos = list(ent.pos)
            positions.append(pos)
        
        minpos = Vector3(min(positions))
        maxpos = Vector3(max(positions))
        median = (minpos + maxpos) / 2
        #print positions
        #print "Min:", minpos
        #print "Max:", minpos
        #print "Median:", median
        
        return median.x, median.y, median.z
        
    def hideManipulator(self):
        #r.logInfo("hiding manipulator")
        if self.usesManipulator:
            try: #XXX! without this try-except, if something is selected, the viewer will crash on exit
                #print "Hiding arrows!"
                if self.manipulator is not None:
                    self.manipulator.scale = 0.0, 0.0, 0.0 #ugly hack
                    self.manipulator.pos = 0.0, 0.0, 0.0 #another ugly hack
                
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
                    self.grabbed = True
                    if submeshid == 3 or (u != 0.0 and u < 0.421875):
                        print "arrow is blue"
                        self.grabbed_axis = AXIS_BLUE
                    elif submeshid == 2 or (u != 0.0 and u < 0.70703125):
                        print "arrow is green"
                        self.grabbed_axis = AXIS_GREEN
                    elif submeshid == 1 or (u != 0.0 and u <= 1.0):
                        print "arrow is red"
                        self.grabbed_axis = AXIS_RED
                    else:
                        print "arrow got screwed..."
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
            entpos = Vector3(ent.pos)#Vector3(ent.pos)
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
            
            #~ id = raycast_results[0]
            #~ ent = r.getEntity(id)
            #~ num = ent.mesh.GetNumMaterials()
            submeshid = raycast_results[-3]
            if submeshid > 0:
                name =  self.MATERIALNAMES[submeshid] + str("_hi")
                self.manipulator.mesh.SetMaterial(submeshid, name)
                self.highlightedSubMesh = submeshid

    def resethighlight(self):
        if self.usesManipulator and self.highlightedSubMesh is not None:
            whee = {
                0: "asd", 
                1: "resed", 
                2: "resed2", 
                3: "resed3"
            }
            name = self.MATERIALNAMES[self.highlightedSubMesh]
            self.manipulator.mesh.SetMaterial(self.highlightedSubMesh, name)
            self.highlightedSubMesh = None
        
class MoveManipulator(Manipulator):
    NAME = "MoveManipulator"
    #MANIPULATOR_MESH_NAME = "axes.mesh"

    def _manipulate(self, ent, amountx, amounty, lengthx, lengthy):
        if self.grabbed:
            rightvec = Vector3(r.getCameraRight())
            upvec = Vector3(r.getCameraUp())
            pos = list(ent.pos)
            #print rightvec[self.manipulatorGrabbed_axis], rightvec
            if self.grabbed_axis == AXIS_BLUE:
                mov = lengthy
                #print mov, pos[self.manipulatorGrabbed_axis],
                pos[self.grabbed_axis] -= mov
                #print pos[self.manipulatorGrabbed_axis]
            else:
                mov = lengthx 
                div = abs(rightvec[self.grabbed_axis])
                if div == 0:
                    div = 0.01 #not the best of ideas but...
                mov *= rightvec[self.grabbed_axis]/div
                #print mov, pos[self.manipulatorGrabbed_axis],
                pos[self.grabbed_axis] += mov
            
            #print pos[self.manipulatorGrabbed_axis]
            
            ent.pos = pos[0], pos[1], pos[2]
            #self.manipulator.pos = pos[0], pos[1], pos[2]
        
class ScaleManipulator(Manipulator):
    NAME = "ScaleManipulator"
    #MANIPULATOR_MESH_NAME = "axes.mesh"
    
    def _manipulate(self, ent, amountx, amounty, lengthx, lengthy):
        if self.grabbed:
            scale = list(ent.scale)
            rightvec = Vector3(r.getCameraRight())
            upvec = Vector3(r.getCameraUp())
            
            if self.grabbed_axis == AXIS_BLUE:
                mov = lengthy
                scale[self.grabbed_axis] -= mov
            else:
                mov = lengthx
                div = abs(rightvec[self.grabbed_axis])
                if div == 0:
                    div = 0.01 #not the best of ideas but...
                mov *= rightvec[self.grabbed_axis]/div
                scale[self.grabbed_axis] += mov
           
            ent.scale = scale[0], scale[1], scale[2]
            self.controller.updateSelectionBox(ent) 
            qprim = ent.prim
            if qprim is not None:
                children = qprim.GetChildren()
                for child_id in children:
                    child = r.getEntity(int(child_id))
                    child.scale = scale[0], scale[1], scale[2]
            
class FreeMoveManipulator(Manipulator):
    NAME = "FreeMoveManipulator"
    USES_MANIPULATOR = False
    
    def _manipulate(self, ent, amountx, amounty, lengthx, lengthy):
        #freemove
        #r.logInfo("_manipulate")
        rightvec = Vector3(r.getCameraRight())
        upvec = Vector3(r.getCameraUp())
        changevec = (amountx * rightvec) - (amounty * upvec)
        #print changevec.length
        entpos = Vector3(ent.pos)
        newpos = entpos + changevec
        ent.pos = newpos.x, newpos.y, newpos.z
        
class RotationManipulator(Manipulator):
    NAME = "RotationManipulator"
    MANIPULATOR_MESH_NAME = "rotate1.mesh"
    
    MANIPULATORORIENTATION = 1, 0, 0, 1
    
    MATERIALNAMES = {
        0: "asd",  #shodows?
        1: "resed", 
        2: "resed2", 
        3: "resed3"
    }
                        
    def _manipulate(self, ent, amountx, amounty, lengthx, lengthy):
        if self.grabbed and self.grabbed_axis is not None:
            #~ print "rotating...", self.grabbed_axis
            #~ print "amounts: ", amountx, amounty
            #~ print "lengths: ", lengthx, lengthy
            
            rightvec = Vector3(r.getCameraRight())
            upvec = Vector3(r.getCameraUp())
            euler = list(quat_to_euler(ent.orientation))

            if self.grabbed_axis == AXIS_GREEN: #rotate z-axis
                #~ print "green axis", self.grabbed_axis, euler
                mov = amountx * 100
                euler[2] += mov
            elif self.grabbed_axis == AXIS_BLUE: #rotate x-axis
                #~ print "blue axis", self.grabbed_axis, euler
                mov = amountx * 100
                euler[1] -= mov
            elif self.grabbed_axis == AXIS_RED: #rotate y-axis
                #~ print "red axis", self.grabbed_axis, euler
                mov = amounty * 100
                euler[0] += mov 
            #~ print mov
            #~ print euler, mov
            ort = euler_to_quat(euler)
            #~ print ort
            ent.orientation = ort
            
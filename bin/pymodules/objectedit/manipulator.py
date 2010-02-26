import rexviewer as r
from vector3 import Vector3 #for view based editing calcs now that Vector3 not exposed from internals

import math

AXIS_X = 0
AXIS_Y = 1
AXIS_Z = 2
    
class Manipulator:
    NAME = "Manipulator"
    MANIPULATOR_MESH_NAME = "axes.mesh"
    USES_MANIPULATOR = True
    
    def __init__(self, creator):
        self.controller = creator;
        self.manipulator = None
        self.grabbed_axis = None
        self.grabbed = False
        self.usesManipulator = self.USES_MANIPULATOR    
        self.manipulator = None

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
            self.manipulator.orientation = 0, 0, 0, 1
    
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
                    u = results[-2]
                    v = results[-1]
                    #print "ARROW and UV", u, v
                    self.grabbed = True
                    if u < 0.421875:
                        #print "arrow is blue / z"
                        self.grabbed_axis = AXIS_Z
                    elif u < 0.70703125:
                        #print "arrow is green / y"
                        self.grabbed_axis = AXIS_Y
                    elif u <= 1.0:
                        #print "arrow is red / x"
                        self.grabbed_axis = AXIS_X
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
            
            if self.usesManipulator:
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

class MoveManipulator(Manipulator):
    NAME = "MoveManipulator"
    #MANIPULATOR_MESH_NAME = "axes.mesh"

    def _manipulate(self, ent, amountx, amounty, lengthx, lengthy):
        if self.grabbed:
            rightvec = Vector3(r.getCameraRight())
            upvec = Vector3(r.getCameraUp())
            pos = list(ent.pos)
            #print rightvec[self.manipulatorGrabbed_axis], rightvec
            if self.grabbed_axis == AXIS_Z:
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
            
            if self.grabbed_axis == AXIS_Z:
                mov = lengthy
                scale[self.grabbed_axis] -= mov
            else:
                mov = lengthx
                div = abs(rightvec[self.grabbed_axis])
                if div == 0:
                    div = 0.01 #not the best of ideas but...
                mov *= rightvec[self.grabbed_axis]/div
                scale[self.grabbed_axis] += mov
           
            ent.scale = scale[0], scale[1],scale[2]
            self.controller.updateSelectionBox()  
            
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
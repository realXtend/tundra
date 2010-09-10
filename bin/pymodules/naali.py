"""namespace config, 'cause the c++ side doesn't do it too nicely"""

from __main__ import _naali
#from _naali import *

#XXX do we actually want these style changes,
#or is it better to just call the slots directly
# module methods
runjs = _naali.RunJavascriptString

def getScene(name):
    return _naali.GetScene(name)
    
def getDefaultScene():
    #XXX should use framework GetDefaultWorldScene
    return getScene("World")

def createInputContext(name, priority = 100):
    return _naali.CreateInputContext(name, priority)

# module variables
renderer = _naali.GetRenderer()
worldlogic = _naali.GetWorldLogic()
inputcontext = _naali.GetInputContext()
mediaplayerservice = _naali.GetMediaPlayerService()

class Entity:
    def __init__(self, qent):
        self.qent = qent
        
    #note: it's possible to write a getter class that is reused for all these,
    #so per component we just define 'placeable: EC_OgrePlaceable' or so.
    #but these two copypasted here first as a test while experimenting with this whole idea
    def get_placeable(self):
        p = self.qent.GetComponentRaw("EC_OgrePlaceable")
        if p is None:
            raise AttributeError, "No placeable component"
        return p
    placeable = property(get_placeable)
    
    def get_camera(self):
        c = self.qent.GetComponentRaw("EC_OgreCamera")
        if c is None:
            raise AttributeError, "No camera component"
        return c
    camera = property(get_camera)
    
def getEntity(entid):
    qent = getScene("World").GetEntityRaw(entid)
    if qent is None:
        raise ValueError, "No entity with id %d" % entid
    return Entity(qent)

#helper funcs to hide api indirections/inconsistenties that were hard to fix,
#-- these allow to remove the old corresponding hand written c funcs in pythonscriptmodule.cpp
import rexviewer as r
def _getAsPyEntity(qentget, errmsg):
    qent = qentget()
    if qent is not None:
        #print qent.Id
        pyent = r.getEntity(qent.Id)
        #print pyent, pyent.id
        return pyent
    else:
        raise ValueError, errmsg

def getUserAvatar():
    return _getAsPyEntity(worldlogic.GetUserAvatarEntityRaw, "No avatar. No scene, not logged in?")
        
def getCamera():
    return _getAsPyEntity(worldlogic.GetCameraEntityRaw, "No default camera. No scene?")
        
    
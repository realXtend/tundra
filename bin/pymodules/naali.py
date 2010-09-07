"""namespace config, 'cause the c++ side doesn't do it too nicely"""

from __main__ import _naali
#from _naali import *

#XXX do we actually want these style changes,
#or is it better to just call the slots directly
# module methods
runjs = _naali.RunJavascriptString
getCamera = _naali.GetCamera
getCameraEntity = _naali.GetCameraEntity

def getScene(name):
    return _naali.GetScene(name)

# module variables
renderer = _naali.GetRenderer()
worldlogic = _naali.GetWorldLogic()
inputcontext = _naali.GetInputContext()
mediaplayerservice = _naali.GetMediaPlayerService()

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
        
    
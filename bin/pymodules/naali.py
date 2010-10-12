"""namespace config, 'cause the c++ side doesn't do it too nicely"""

from __main__ import _pythonscriptmodule
from __main__ import _naali
import rexviewer as r #the old module is still used , while porting away from it
#from _naali import *

#XXX do we actually want these style changes,
#or is it better to just call the slots directly
# module methods
runjs = _pythonscriptmodule.RunJavascriptString

def getScene(name):
    return _pythonscriptmodule.GetScene(name)
    
def getDefaultScene():
    #XXX should use framework GetDefaultWorldScene
    return getScene("World")
    
def createEntity(comptypes = []):
    s = getDefaultScene()
    ent = s.CreateEntityRaw(0, comptypes) #0 apparently means it assigns NextFreeId
    return ent

def createMeshEntity(meshname, raycastprio=1):
    ent = createEntity(["EC_Placeable", "EC_Mesh"])
    ent.placeable.SelectPriority = raycastprio
    ent.mesh.SetPlaceable(ent.placeable)
    ent.mesh.SetMesh(meshname)
    return ent

#XXX check how to do with SceneManager
def removeEntity(entity):
    r.removeEntity(entity.Id)

def createInputContext(name, priority = 100):
    return _pythonscriptmodule.CreateInputContext(name, priority)

# module variables
renderer = _pythonscriptmodule.GetRenderer()
worldlogic = _pythonscriptmodule.GetWorldLogic()
inputcontext = _pythonscriptmodule.GetInputContext()
mediaplayerservice = _pythonscriptmodule.GetMediaPlayerService()
frame = _naali.GetFrame()
console = _naali.Console()
input = _naali.GetInput()
audio = _naali.Audio()
ui = _naali.UiService() #the UI core object does not implement the old uiservice stuff yet
framework = _naali

# Returns EntityAction pointer by the name
def action(self, name):
    return self.qent.Action(name)
    
def getEntity(entid):
    qent = getScene("World").GetEntityRaw(entid)
    if qent is None:
        raise ValueError, "No entity with id %d" % entid
    return qent

#helper funcs to hide api indirections/inconsistenties that were hard to fix,
#-- these allow to remove the old corresponding hand written c funcs in pythonscriptmodule.cpp
def _getAsPyEntity(qentget, errmsg):
    qent = qentget()
    if qent is not None:
        #print qent.Id
        return qent
    else:
        raise ValueError, errmsg

def getUserAvatar():
    return _getAsPyEntity(worldlogic.GetUserAvatarEntityRaw, "No avatar. No scene, not logged in?")
        
def getCamera():
    return _getAsPyEntity(worldlogic.GetCameraEntityRaw, "No default camera. No scene?")

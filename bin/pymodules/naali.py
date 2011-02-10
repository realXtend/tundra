"""namespace config, 'cause the c++ side doesn't do it too nicely"""

from __main__ import _pythonscriptmodule
from __main__ import _naali
import rexviewer as r #the old module is still used , while porting away from it
import sys #for stdout redirecting
#from _naali import *

# for PythonQt.private.AttributeChange
import PythonQt

#\todo: do we have version num info somewhere?
#version = XXX

class Logger:
    def __init__(self):
        self.buf = ""

    def write(self, msg):
        self.buf += msg
        while '\n' in self.buf:
            line, self.buf = self.buf.split("\n", 1)
            r.logInfo(line)

sys.stdout = Logger()

#XXX do we actually want these style changes,
#or is it better to just call the slots directly
# module methods
runjs = _pythonscriptmodule.RunJavascriptString

def getScene(name):
    return _pythonscriptmodule.GetScene(name)
    
def getDefaultScene():
    return _naali.DefaultScene()
    
def createEntity(comptypes = [], localonly = False, sync = True, temporary = False):
    s = getDefaultScene()
    # create entity
    if localonly:
        ent = s.CreateEntityLocalRaw(comptypes)
    else:
        ent = s.CreateEntityRaw(0, comptypes, PythonQt.private.AttributeChange.Replicate, sync)
    # set temporary
    if temporary:
        ent.SetTemporary(True)
    # create components
    for comptype in comptypes:
        comp = ent.GetOrCreateComponentRaw(comptype)
        if temporary:
            comp.SetTemporary(True)
    s.EmitEntityCreatedRaw(ent)
    return ent

def createMeshEntity(meshname, raycastprio=1, additional_comps = [], localonly = False, sync = True, temporary = False):
    components = ["EC_Placeable", "EC_Mesh", "EC_Name"] + additional_comps
    ent = createEntity(components, localonly, sync, temporary)
    ent.placeable.SelectPriority = raycastprio
    ent.mesh.SetPlaceable(ent.placeable)
    ent.mesh.SetMesh(meshname)
    return ent

#XXX check how to do with SceneManager
def removeEntity(entity):
    r.removeEntity(entity.Id)
    
# using the scene manager, note above
def deleteEntity(entity):
    s = getDefaultScene()
    s.DeleteEntityById(entity.Id)

def createInputContext(name, priority = 100):
    return _pythonscriptmodule.CreateInputContext(name, priority)

# module variables
renderer = _pythonscriptmodule.GetRenderer()
worldlogic = _pythonscriptmodule.GetWorldLogic()
inputcontext = _pythonscriptmodule.GetInputContext()
mediaplayerservice = _pythonscriptmodule.GetMediaPlayerService()

# the comms service is optional. If it's not present we'll leave it
# undefined.
try:
    communicationsservice = _pythonscriptmodule.GetCommunicationsService()
except AttributeError:
    pass

frame = _naali.GetFrame()
console = _naali.Console()
input = _naali.GetInput()
audio = _naali.Audio()
ui = _naali.UiService() #the UI core object does not implement the old uiservice stuff yet
uicore = _naali.Ui()
framework = _naali
debug = _naali.Debug()

# Returns EntityAction pointer by the name
def action(self, name):
    return self.qent.Action(name)
    
def getEntity(entid):
    qent = getDefaultScene().GetEntityRaw(entid)
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
    return _getAsPyEntity(_pythonscriptmodule.GetActiveCamera, "No default camera. No scene?")

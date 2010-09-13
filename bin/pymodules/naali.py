"""namespace config, 'cause the c++ side doesn't do it too nicely"""

from __main__ import _naali
import rexviewer as r #the old module is still used , while porting away from it
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
    
def createEntity(comptypes = []):
    s = getDefaultScene()
    newid = s.NextFreeId()
    ent = s.CreateEntityRaw(newid, comptypes)
    return Entity(ent)

#def createMeshEntity(meshname):
#    ent = createEntity()

    #XXX ugly workaround
    #oldpyent = r.getEntity(ent.id)
    #oldpyent.createComponent("EC_OgreMesh") #sets placeable too

    #XXX wasn't possible yet. lead into research about adding QPointer support to PythonQt internals etc
    #ent = naali.createEntity(["EC_OgrePlaceable", "EC_OgreMesh"])
    #ent.mesh.SetPlaceable(ent.placeable) #wants a boost shared_ptr, which we don't have :/
    #ent.mesh.SetMesh(meshname)

def createInputContext(name, priority = 100):
    return _naali.CreateInputContext(name, priority)

# module variables
renderer = _naali.GetRenderer()
worldlogic = _naali.GetWorldLogic()
inputcontext = _naali.GetInputContext()
mediaplayerservice = _naali.GetMediaPlayerService()

class Entity:
    compnames = {
        'placeable': 'EC_OgrePlaceable',
        'mesh': 'EC_OgreMesh',
        'camera': 'EC_OgreCamera',
        'animationcontroller': 'EC_OgreAnimationController',
        'highlight': 'EC_Highlight',
        'touchable': 'EC_Touchable',
        }   
 
    def __init__(self, qent):
        self.id = qent.Id

        """The Scene::Entity exposed by Qt mechanism. Public, ok to use directly as well."""
        self.qent = qent


    def getComponent(self, typename, idname=None):
        """To allow getting components for which there is no shortcut in py Entity helper"""

        if idname is None: #the right qt slot is found based on the amount of params given
            return self.qent.GetComponentRaw(typename)
        else:
            return self.qent.GetComponentRaw(typename, idname)

    def getDynamicComponent(self, idname):
        return self.getComponent("EC_DynamicComponent", idname)
        
    def __getattr__(self, name):
        if name in Entity.compnames:
            fullname = Entity.compnames[name]
            comp = self.qent.GetComponentRaw(fullname)
            if comp is None: 
                raise AttributeError, "The entity does not have a %s component" % fullname
            return comp
        raise AttributeError

    def createComponent(self, typename):
        oldpyent = r.getEntity(self.id)
        oldpyent.createComponent(typename)

def getEntity(entid):
    qent = getScene("World").GetEntityRaw(entid)
    if qent is None:
        raise ValueError, "No entity with id %d" % entid
    return Entity(qent)

#helper funcs to hide api indirections/inconsistenties that were hard to fix,
#-- these allow to remove the old corresponding hand written c funcs in pythonscriptmodule.cpp
#.. and now am working towards removal of PyEntity.cpp too
def _getAsPyEntity(qentget, errmsg):
    qent = qentget()
    if qent is not None:
        #print qent.Id
        pyent = Entity(qent)
        #print pyent, pyent.id
        return pyent
    else:
        raise ValueError, errmsg

def getUserAvatar():
    return _getAsPyEntity(worldlogic.GetUserAvatarEntityRaw, "No avatar. No scene, not logged in?")
        
def getCamera():
    return _getAsPyEntity(worldlogic.GetCameraEntityRaw, "No default camera. No scene?")


# Import PythonQt
import PythonQt

# Import PythonScriptModule and Tundra SDK
from __main__ import _pythonscriptmodule
from __main__ import _tundra

""" Tundra SDK APIs. Tries to be similar as you would call Framework
    hence they are functions and not public members. We get to a situation where
    C++    = framework->Asset()->RequestAsset(...)
    Python = import tundra as tundra
             tundra.Asset().RequestAsset(...) """

def Framework():
    return _tundra
    
def Frame():
    return _tundra.Frame()
    
def Scene():
    return _tundra.Scene()

def Asset():
    return _tundra.Asset()
    
def Ui():
    return _tundra.Ui()
    
def Audio():
    return _tundra.Audio()
    
def Input():
    return _tundra.Input()
    
def Console():
    return _tundra.Console()
    
""" TundraProtocolModule: Client and Server """
    
def IsServer():
    return Server().IsAboutToStart()
    
def IsClient():
    return not IsServer()
    
def Client():
    return _pythonscriptmodule.GetClient()
    
def Server():
    return _pythonscriptmodule.GetServer()
    
""" OgreRennderingModule: Renderer """
    
def Renderer():
    return _pythonscriptmodule.GetRenderer()
    
""" Logging functions """
    
def LogInfo(m):
    _pythonscriptmodule.PythonPrintLog("INFO", m)
    
def LogWarning(m):
    _pythonscriptmodule.PythonPrintLog("WARNING", m)
    
def LogError(m):
    _pythonscriptmodule.PythonPrintLog("ERROR", m)
    
def LogDebug(m):
    _pythonscriptmodule.PythonPrintLog("DEBUG", m)
    
def LogFatal(m):
    _pythonscriptmodule.PythonPrintLog("FATAL", m)

""" Python helper """

def Helper():
    return _helper
    
""" Helper class to capsulate some special helpers for Tundar python code.
    Usually because some slots are still inconvinient to call from the original APIs.
    For example: Some things returning boost shared ptr:s that python hold to increase the ref
    count so the object dies immidiately. Or AttributeChange.Replicate is not obvious to everyone
    from PythonQt.private.AttributeChange. """
    
class Helper:

    def __init__(self):
        pass
        
    """ Creates a InputContext, PythonScriptModule will hold a shared ref of untill
        it is unloaded. Remove when creating Input Context from InputAPI wont return a shared ptr
        that dies after returning. """
    def CreateInputContext(self, name, priority = 100):
        return _pythonscriptmodule.CreateInputContext(name, priority)
        
    """ Creates a new entity with various input params. Remove this helper 
        once creating replicated and local entities is easy enough for python. """
    def CreateEntity(self, comptypes = [], localonly = False, sync = True, temporary = False):
        s = Scene().GetDefaultSceneRaw()
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
            ent.GetOrCreateComponent(comptype)
            # No more *Raw() funcs, it returns boost shaerd ptr, find a way to convert these!
            #comp = ent.GetOrCreateComponentRaw(comptype)
            #if temporary:
            #    comp.SetTemporary(True)
        s.EmitEntityCreatedRaw(ent)
        return ent

_helper = Helper()

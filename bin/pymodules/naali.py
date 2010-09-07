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
inputcontext = _naali.GetInputContext()
mediaplayerservice = _naali.GetMediaPlayerService()

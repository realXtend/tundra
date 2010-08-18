"""namespace config, 'cause the c++ side doesn't do it too nicely"""

from __main__ import _naali

n = _naali

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


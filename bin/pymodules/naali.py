"""namespace config, 'cause the c++ side doesn't do it too nicely"""

from __main__ import _naali
#from _naali import *

renderer = _naali.GetRenderer()
runjs = _naali.RunJavascriptString
inputcontext = _naali.GetInputContext()
mediaplayerservice = _naali.GetMediaPlayerService()
def getScene(name):
    return _naali.GetScene(name)

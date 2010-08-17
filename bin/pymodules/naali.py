"""namespace config, 'cause the c++ side doesn't do it too nicely"""

from __main__ import _naali
#from _naali import *

renderer = _naali.GetRenderer()
runjs = _naali.RunJavascriptString
inputcontext = _naali.GetInputContext()
player_service = _naali.GetPlayerService()
def getScene(name):
    return _naali.GetScene(name)

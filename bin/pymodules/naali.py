"""namespace config, 'cause the c++ side doesn't do it too nicely"""

from __main__ import _naali

renderer = _naali.GetRenderer()
runjs = _naali.RunJavascriptString

def getScene(name):
    return _naali.GetScene(name)

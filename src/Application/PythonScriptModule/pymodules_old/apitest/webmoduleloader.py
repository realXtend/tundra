import componenthandler
import circuits
import imp
import urllib

TRUSTEDBASEURL = "http://localhost:82/"

webmodules = {} #modulename : moduleobject, to avoid loading same several times

def loadwebmodule(srcurl):
    if srcurl not in webmodules:
        url = TRUSTEDBASEURL + srcurl
        filepath = urllib.urlretrieve(url)[0]
        f = open(filepath)
        mod = imp.load_module("webmod", f, 'webmod.py', ('.py', 'U', 1))
        webmodules[srcurl] = mod
    else:
        mod = webmodules[srcurl]
    return mod

class WebPythonmoduleLoader(circuits.BaseComponent):
    def __init__(self, entity, comp, changetype):
        circuits.BaseComponent.__init__(self)
        comp.connect("OnChanged()", self.onChanged)
        self.comp = comp

    def onChanged(self): #was this the current or old version?
        srcurl = self.comp.GetAttribute("srcurl")
        print srcurl
        mod = loadwebmodule(srcurl)

#componenthandler.register("pythonmodule", WebPythonmoduleLoader)

import componenthandler
import circuits
import imp
import urllib

def loadwebmodule(srcurl):
    filepath = urllib.urlretrieve(srcurl)[0]
    f = open(filepath)
    mod = imp.load_module("webmod", f, 'webmod.py', ('.py', 'U', 1))
    return mod

class WebPythonmoduleLoader(circuits.BaseComponent):
    ADDMENU = False

    def __init__(self, entity, comp, changetype):
        circuits.BaseComponent.__init__(self)
        comp.connect("OnChanged()", self.onChanged)
        self.comp = comp

    def onChanged(self): #was this the current or old version?
        srcurl = self.comp.GetAttribute("srcurl")
        print srcurl
        mod = loadwebmodule(srcurl)

componenthandler.register("pythonmodule", WebPythonmoduleLoader)

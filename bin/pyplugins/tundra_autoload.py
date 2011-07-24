
""" The modules that are to be loaded when the viewer starts.
    add your module *class* (the circuits Component) to a .ini file in this directory. """

import sys
import os
import traceback
    
import tundra as tundra

from glob import glob
from ConfigParser import ConfigParser

def freshimport(modulename): #aka. load_or_reload
    mod = None
    if sys.modules.has_key(modulename):
        try:
            mod = reload(sys.modules[modulename])
        except Exception, e:
            msg = "Problem reloading plugin '%s'" % modulename
            tundra.LogError(msg)
            tundra.LogError(str(e))
            tundra.LogError(traceback.format_exc())

    else:
        try:
            mod = __import__(modulename, globals(), locals(), [""])
        except:
            tundra.LogError("Couldn't load %s" % modulename)
            tundra.LogError(traceback.format_exc())

    return mod
    
def read_inis():
    thisdir = os.path.split(os.path.abspath(__file__))[0]
    for inifile in glob(os.path.join(thisdir, "*.ini")):
        tundra.LogInfo("** " + inifile)
        cp = ConfigParser()
        cp.read([inifile])
        for s in cp.sections():
            cfdict = dict(cp.items(s))
            try:
                modname, compname = s.rsplit('.', 1)
            except ValueError:
                tundra.LogError("Bad config section in " + inifile + ": " + s)
                continue

            yield modname, compname, cfdict

modules = []

tundra.LogInfo("Loading Python plugins")
for modname, compname, cfg in read_inis():
    m = freshimport(modname)
    if m is not None: #loaded succesfully. if not, freshload logged traceback
        c = getattr(m, compname)
        modules.append((c, cfg))

def load(circuitsmanager):
    for klass, cfg in modules:
        #tundra.LogInfo("[Python::AutoLoad] -- Loading:" + klass)
        try:
            if cfg:
                modinst = klass(cfsection=cfg)
            else:
                modinst = klass()
        except Exception, exc:
            tundra.LogError("Failed to instantiate pymodule %s" % klass)
            tundra.LogError(traceback.format_exc())
        else:
            circuitsmanager += modinst # Equivalent to: tm.register(m)
            
def unload():
    for klass, cfg in modules:
        print "Unloading", klass

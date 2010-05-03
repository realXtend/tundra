import sys
import traceback

try:
    import rexviewer as r
except:
    import mockviewer as r

def freshimport(modulename): #aka. load_or_reload
    mod = None
    fromlist = []
    if '.' in modulename:
        fromlist.append("") #to change __import__ to give submodule #modulename.split('.')[0])
    if sys.modules.has_key(modulename):
        try:
            mod = reload(sys.modules[modulename])
        except Exception, e:
            msg = "Problem reloading plugin '%s'" % modulename
            r.logInfo(msg)
            r.logInfo(str(e))
            r.logInfo(traceback.format_exc())

    else:
        try:
            mod = __import__(modulename, globals(), locals(), fromlist)
        except:
            r.logInfo("couldn't load %s" % modulename)
            r.logInfo(traceback.format_exc())

    return mod

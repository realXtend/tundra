import sys

try:
    import rexviewer as r
except:
    import mockviewer as r

def freshimport(modulename): #aka. load_or_reload
    fromlist = []
    if '.' in modulename:
        fromlist.append("") #to change __import__ to give submodule #modulename.split('.')[0])
    if sys.modules.has_key(modulename):
        try:
            m = reload(sys.modules[modulename])
        except Exception, e:
            msg = "Problem reloading plugin '%s'" % modulename
            r.logError(msg)
            r.logError(e)
            r.logError(traceback.format_exc())
            return None

    else:
        m = __import__(modulename, globals(), locals(), fromlist)

    return m

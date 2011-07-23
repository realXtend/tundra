"""
a logger object that can be given to circuits debugger,
which writes to Naali log instead of stderr
"""

import rexviewer as r

class NaaliLogger:
    def error(self, s):
        r.logInfo(s) #logError wasn't exposed - also errors in 3rd party plugins are not naali errors, but something that should info for the authors?

    def debug(self, s):
        r.logInfo(s) #wanted this to show in release mode too, to be able to debug py stuff when just developing on a prebuilt release. Debug(s)

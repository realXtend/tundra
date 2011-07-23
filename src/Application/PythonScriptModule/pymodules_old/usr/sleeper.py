"""the viewer currently doesn't have a framelimiter,
'cause the ones that we had earlier didn't work well after the qt integration.
this is a little hack to save cpu, before timers return to the core"""

import time
from circuits import Component

#n = 10
#print "Sleeper ver:", n

class Sleeper(Component):
    def update(self, t):
        time.sleep(0.01)
#        print n

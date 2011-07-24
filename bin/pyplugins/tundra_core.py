
import tundra as tundra

""" Make sure we have python sdtlib and circuits present """

try:
    import os
except:
    tundra.LogError("Python stdlib not there! [Windows OS: Are you sure you have /pyplugins/python/Python26.zip present?]")
    
try:
    from circuits import handler, Event, Component, Manager, Debugger
    from circuits.core.events import Started
except:
    tundra.LogError("Could not import needed circuits components!")

""" Tundra circuits module manager """

class ModuleManager:

    def __init__(self):
        self.firstRun = True
        self.start()
        
    def start(self): 
        # List our own channels
        ignoreNames = ['update', 'on_exit']
        ignoreChannels = [('*', n) for n in ignoreNames]
        
        # Init circuits
        self.circuitsDebugger = Debugger(IgnoreChannels = ignoreChannels, logger=TundraLogger())
        self.circuitsManager = Manager() + self.circuitsDebugger
        
        # Do autoloading
        """
        if self.firstRun:
            print "autoload first run"
            # Get autoload module
            import tundra_autoload
            self.firstRun = False
        else:
            print "autoload second run??"
            # Make python reload autoload module
            import tundra_autoload
            tundra_autoload = reload(tundra_autoload)
        """
        try:
            import tundra_autoload
            try:
                tundra_autoload.load(self.circuitsManager)
            except:
                pass
        except:
            tundra.LogError("Could not import needed tundra_autoload.py")
        
        # Some components need this
        self.circuitsManager.push(Started(self.circuitsManager, None))

    def exit(self):
        tundra.LogInfo("[Python::ModuleManager] Exiting")
        import tundra_autoload
        tundra_autoload.unload()
        # Is this needed? "Crashes" Py_Finalize.
        #self.sendEvent(Exit(), "on_exit")
        self.circuitsManager.stop()
        while self.circuitsManager: 
            self.circuitsManager.flush()
            
    def update(self, frametime):
        self.send_event(Update(frametime), "update")
        self.circuitsManager.tick()
        
    def sendEvent(self, event, channel):
        # Push the event to circuits
        tempManager = self.circuitsManager;
        returnValue = tempManager.push(event, channel)
        
        # Circuits components evaluate to false 
        # when have no pending events left
        while tempManager: 
            tempManager.flush()
            
        if not returnValue.errors:
            return True in ret
        return False
        
""" Custom logger for circuits """

class TundraLogger:
    def error(self, s):
        print s
        tundra.LogError(s)

    def debug(self, s):
        print s
        tundra.LogInfo(s)

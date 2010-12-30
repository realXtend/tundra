import time
import os
import threading
import subprocess
import sys
import time
import Queue
import meshhandler


class Debug:
    """ Class for debugging what happens inside process threads """
    def __init__(self, fname):
        self.fname = fname
        self.debug_on = True # set True to print debugs
    def writeDebug(self, s):
        if self.debug_on==True:
            f = open(self.fname, "a")
            f.write(s)
            f.write('\n')
            f.flush()
            f.close()

            
class SomeHeavyprocess(Debug):
    """ Testing class for running process """
    def __init__(self, arg):
        self.state = "init"
        Debug.__init__(self, "_shp.txt")
    def getState(self):
        return self.state
    def start(self):
        for i in range(0,9):
            time.sleep(0.5)
            self.state = str(i)
            self.writeDebug(str(i))
        time.sleep(1)
        self.state = "end"
        

class MessageHandler(Debug): 
    """ Generic class writing and reading from process stdio, stdin """
    def __init__(self, sequenceTime, input, output, debugtxt):
        Debug.__init__(self, debugtxt)
        self.seq = sequenceTime
        self.input = input
        self.output = output
        self.timer = None
        self.writeDebug("initialized")
        
    def startPoll(self):
        self.writeDebug("startPoll with seq: " + str(self.seq))
        self.timer=threading.Timer(self.seq, self.poll)
        self.timer.start()
        
    def write(self, message):
        self.writeDebug("write message: " + message)
        self.output.write(message)
        self.output.write("\n")
        self.output.flush()
        self.writeDebug("flushed")
        
    def poll(self):
        self.writeDebug("poll : reading line")
        line=self.input.readline()
        self.writeDebug("read line: " + line)
        self.handleMessage(line)
        
    def handleMessage(self, line):
        pass
    
    def loop(self, message):
        if message.startswith("__end__"):
            self.writeDebug("ending")
        else:
            self.writeDebug("restart")
            self.timer=threading.Timer(self.seq, self.poll)
            self.timer.start()
            
        
        
class ProcessReader(MessageHandler):  
    """ read process output """
    def __init__(self, sequenceTime, input, output):
        MessageHandler.__init__(self, sequenceTime, input, output, "_pr.txt")
        self.controller = None

    def poll(self): #override
        self.writeDebug("poll")
        self.output.write("getstate\n")
        self.writeDebug("getstate")
        self.output.flush()
        line=self.input.readline()
        self.writeDebug("readline")
        self.handleMessage(line)
        
    def handleMessage(self, line):
        self.writeDebug("controller handle line")
        self.controller.handle(line)
        if(line.startswith('end')):
            self.output.write("__end__")
            return
        self.writeDebug("call loop")
        self.loop(line)
        

class ProcessMessager(MessageHandler): 
    """ send messages from process """
    def __init__(self, sequenceTime, input, output):
        MessageHandler.__init__(self, sequenceTime, input, output, "_pm.txt")
        self.shp = None
        pass
        
    def poll(self): # override
        self.writeDebug("ProcessMessager : reading line")
        line=self.input.readline()
        self.writeDebug("read line: " + line)
        self.handleMessage(line)
        
        
    def handleMessage(self, line):
        #self.write(line)
        self.writeDebug("handleMessage")
        self.writeDebug(line)
        if(line.startswith("getstate")):
            state = self.shp.getState()
            self.write(state)
            # state = shp.getState()
            # self.writeDebug("Writing, line started with getstate")
            # self.writeDebug(state)
            # self.write(state)
            # output.write(state)
            # output.write("\n")
        self.writeDebug("call loop")
        self.loop(line)
        pass


class LoadProcessHandle: 
    """ Class for storing handle to load process """
    def __init__(self):
        self.file = "pymodules/localscene/loadprocess.py"
        self.p = None
        pass
        
    def startProcess(self, args, controller):
        print "startProcess"
        #self.file = file
        cwd=os.getcwd()
        pypath = cwd + "\\pymodules\\python26_Lib.zip;"
        os.putenv('PYTHONPATH', pypath)
        
        self.p = subprocess.Popen(["python", self.file, args], #env=self.envVars
            shell=False, 
            stdin=subprocess.PIPE, 
            stdout=subprocess.PIPE)
        # --- input/output ---
        input = self.p.stdout # readline()
        output = self.p.stdin # write()
        self.pr = ProcessReader(0.5, input, output)
        self.pr.controller=controller
        print "startPoll"
        self.pr.startPoll()
        pass

        
if __name__ == '__main__':
    filename=sys.argv[1]
    #shp = SomeHeavyprocess(filename)
    mh = meshhandler.MeshHandler(filename)
    # --- input/output ---
    input = sys.stdin # readline()
    output = sys.stdout # write()

    pm = ProcessMessager(0.5, input, output)
    #pm.shp = shp
    pm.shp = mh
    pm.startPoll()
    #shp.start()
    mh.start()
    

        
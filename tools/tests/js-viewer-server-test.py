#!/usr/local/bin/python


##
# TODO:
# - opt params for test scene
# - exit status not working as intended ?
#   - e.g. viewer not found -> still 0
#   - also when gotten from bash with $? still 0
#   - (orginal bash script also returns same values)
##
import os
import os.path
import time
import subprocess
from multiprocessing import Process
import resource
import shutil
import autoreport
from optparse import OptionParser
import config


testName="js-viewer-server-test"

#folder config
scriptDir = config.scriptDir
rexbinDir = config.rexbinDir
testDir = config.testDir
logsDir = config.jvstLogsDir

SCRIPTSUFFIX = ".js"

#define scriptnames
serverScript = logsDir + "/s" + SCRIPTSUFFIX
viewerScript = logsDir + "/v" + SCRIPTSUFFIX

#exitcode files
exitfileViewer = logsDir + "/exitstatus.v"
exitfileServer = logsDir + "/exitstatus.s"
# output files
serverOutput = logsDir + "/s.out"
viewerOutput = logsDir + "/v.out"

testfile = rexbinDir + "/scenes/Avatar/avatar.txml"


def main():
    makePreparations()
    runTest()
    compareErrorCodes()
    printErrors()
    autoreport.autoreport("js-viewer-server-test")

def makePreparations():
    #make logs directory
    if not os.path.exists(logsDir):
        os.makedirs(logsDir)

def runTest():
    #make scripts
    if testfile[-5:] == ".txml":
        makeScripts('server', 'txml')
        makeScripts('viewer', 'txml')
    elif testfile[-3:] == ".js":
        makeScripts('server', 'js')
        makeScripts('viewer', 'js')
    else:
        print("Check testfile, no valid suffix found (.txml or .js supported)")

    os.chdir(rexbinDir)
    
    # os.name is 'posix', 'nt', 'os2', 'mac', 'ce' or 'riscos'
    # linuxstuff() or macStuff()
    if os.name == 'posix' or os.name == 'mac':
        #enable core dump
        resource.setrlimit(resource.RLIMIT_CORE, (-1, -1))

        if testfile[-3:] == ".js":
            p1 = Process(target=posixJsServer, args=(serverScript,))
            p2 = Process(target=posixJsViewer, args=(viewerScript,))
        elif testfile[-5:] == ".txml":
            p1 = Process(target=posixTxmlServer, args=(serverScript,))
            p2 = Process(target=posixTxmlViewer, args=(viewerScript,))
        else:
            print "error"
        #start both server and viewer in processes
        p1.start()
        p2.start()
        running = True

        # test that both processes have terminated
        while running == True:
            if not p1.is_alive() and not p2.is_alive():
                running = False
            else:
                time.sleep(1)

        # move core dump to outputdir
        if os.path.isfile("core"):
            try:
                #if old core file is present in the logsDir
                if os.path.isfile(logsDir + "/core"):
                    #... remove it os new one can be moved there
                    os.remove("core")
                    shutil.move("core", logsDir)
                else:
                    shutil.move("core", logsDir)
            except:
                print "error moving core dump file (file exists in target location ?)"
    # windowsStuff()
    elif os.name == 'nt': # --NOT TESTED--
        todo
        if testfile[-3:] == ".js":
            p1 = Process(target=ntJsServer, args=(serverScript,))
            p2 = Process(target=ntJsViewer, args=(viewerScript,))
        elif testfile[-5:] == ".txml":
            p1 = Process(target=ntTxmlServer, args=(serverScript,))
            p2 = Process(target=ntTxmlViewer, args=(viewerScript,))
        else:
            print "error"
        #start both server and viewer in processes
        p1.start()
        p2.start()
        running = True
        #test that both processes have terminated
        while running == True:
            if not p1.is_alive() and not p2.is_alive():
                running = False
            else:
                time.sleep(1)

    # back to scriptdir
    os.chdir(scriptDir)

### PROCESS FUNCTIONS
### POSIX START
def posixJsServer(serverScript):
    s = "./Tundra --server --headless --run " + serverScript + " 2>&1 | tee " + serverOutput
    #get exitcode and run
    servercode = posixRun(s)
    #write exitcode to file
    with open(exitfileServer, 'w') as e:
        e.write(servercode)

def posixJsViewer(viewerScript):
    v = "./Tundra --headless --run " + viewerScript + " 2>&1 | tee " + viewerOutput
    viewercode = posixRun(v)
    with open(exitfileViewer, 'w') as e:
        e.write(viewercode)

def posixTxmlServer(serverScript):
    s = "./Tundra --server --protocol tcp --headless --file " + testfile + " --run " + serverScript + " 2>&1 | tee " + serverOutput
    servercode = posixRun(s)
    with open(exitfileServer, 'w') as e:
        e.write(str(servercode))

def posixTxmlViewer(viewerScript):
    v = "./Tundra --headless --storage " + os.path.dirname(testfile) + "/ --run " + viewerScript + " 2>&1 | tee " + viewerOutput
    viewercode = posixRun(v)
    with open(exitfileViewer, 'w') as e:
        e.write(str(viewercode))

def posixRun(str):
    retcode = subprocess.call('xterm -e bash -c "' + str + '"', shell=True)
    return retcode
#    try:
##        retcode = subprocess.check_call(str + ";echo $? >>~/asd.txt", shell=True) # still zeros with viewer missing
#    except Exeption as e:
#        print "failsslsls"
#    return retcode
### POSIX END

### NT START
def ntJsServer(serverScript):
    s = "Tundra.exe --headless --run " + serverScript + " > " + serverOutput + "2>&1"
    #get exitcode and run
    servercode = ntRun(s)
    #write exitcode to file
    with open(exitfileServer, 'w') as e:
        e.write(servercode)

def ntJsViewer(viewerScript):
    v = "Tundra.exe --headless --run " + viewerScript + " > " + viewerOutput + "2>&1"
    viewercode = ntRun(v)
    with open(exitfileViewer, 'w') as e:
        e.write(viewercode)

def ntTxmlServer(serverScript):
    s = "Tundra.exe --headless --protocol tcp --file " + testfile + " --run " + serverScript + " > " + serverOutput + "2>&1"
    servercode = ntRun(s)
    with open(exitfileServer, 'w') as e:
        e.write(servercode)

def ntTxmlViewer(viewerScript):
#    v = "viewer.exe --headless --storage " + os.path.dirname(testfile) + "/ --run " + viewerScript + " > " + viewerOutput + "2>&1 | ECHO %ERRORLEVEL%" > asd.txt
    v = "Tundra.exe --headless --storage " + os.path.dirname(testfile) + "/ --run " + viewerScript + " > " + viewerOutput + "2>&1"
    viewercode = ntRun(v)
    with open(exitfileViewer, 'w') as e:
        e.write(viewercode)

def ntRun(str):
#    retcode = subprocess.call('cmd.exe /c /e:on"' + str + '"', shell=True)
    retcode = subprocess.call('cmd.exe /c "' + str + '"', shell=True)
    return retcode
### NT END

### MAC START
#->posix
### MAC END

def makeScripts(case, suffix):
    EXITDELAY = 'var d = frame.DelayedExecute(15.0); d.Triggered.connect(function (time) { print("exiting!"); framework.Exit(); });'
    DOLOGIN = 'var d = frame.DelayedExecute(0.5); d.Triggered.connect(function (time) { client.Login("localhost",2345,"foo"," ","tcp");});'

    if case == "server":
        fileName = serverScript

        with open(fileName,'w') as ss:
            ss.write(EXITDELAY)
            
        if suffix == "js":
            with open(fileName,'a') as ss:
                ss.write(testfile)

    elif case == "viewer":
        fileName = viewerScript

        with open(fileName,'w') as vs:
            vs.write(EXITDELAY)
            vs.write(DOLOGIN)

# compare viewer and server errorcodes
def compareErrorCodes():
    viewer = ""
    server = ""

    # read files
    with open(exitfileViewer, 'r') as v:
        viewer = v.read()
    with open(exitfileServer, 'r') as s:
        server = s.read()

    print "exitstatus.s: " + server
    print "exitstatus.v: " + viewer
    print "Test outcome:",
    if viewer == str(0) and server == str(0):
        print "Success"
    else:
        print "Failure"

def printErrors():
    outputFiles = serverOutput, viewerOutput
    hit="Error:"

    for i in range(0, len(outputFiles)):
        for line in open(outputFiles[i]):
            if hit in line:
                print outputFiles[i] +": " +line
    

if __name__ == "__main__":
    parser = OptionParser()
    parser.add_option("-f", "--file", dest="file")
    (options, args) = parser.parse_args()
    if options.file:
        testfile = options.file
    main()

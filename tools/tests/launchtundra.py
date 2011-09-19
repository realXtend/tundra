#!/usr/local/bin/python

#import
import os
import os.path
import subprocess
from optparse import OptionParser
import config
import autoreport

# folder config
scriptDir = config.scriptDir
rexbinDir = config.rexbinDir
testDir = config.testDir
logsDir = config.tundraLogsDir

# output files
serverOutput = logsDir + "/s.out"
viewerOutput = logsDir + "/v.out"
# output result!!!

testName = "launchtundra"

param = ""

def main():
    makePreparations()
    os.chdir(rexbinDir)
    runTundra(param)
    os.chdir(scriptDir)
    autoreport.autoreport(testName)

def makePreparations():
    if not os.path.exists(logsDir):
        os.makedirs(logsDir)

def runTundra(param):
    #os.name options: 'posix', 'nt', 'os2', 'mac', 'ce' or 'riscos'
    if os.name == 'posix' or os.name == 'mac':
        if ("--server" in param):
            t = "./Tundra " + param + " 2>&1 | tee " + serverOutput
        else:
            t = "./Tundra " + param + " 2>&1 | tee " + viewerOutput
        subprocess.call(t, shell=True)

    #elif os.name == 'nt':	#NOT IMPLEMENTED
        #windowsStuff

if __name__ == "__main__":
    parser = OptionParser()
    parser.add_option("-p", "--parameters", dest="param")
    (options, args) = parser.parse_args()
    if options.param:
        param = options.param
    main()

#!/usr/local/bin/python

##
# TODO:
# 
##

#import
import os
import time
import shutil
import subprocess
import getpass
import glob
import autoreport
from multiprocessing import Process
from optparse import OptionParser
import config

#variables
numberOfRuns = 2
numberOfClients = 2

testName = "avatar-test"

#folder config
scriptDir = config.scriptDir
rexbinDir = config.rexbinDir
testDir = config.testDir
logsDir = config.avatarLogsDir
wiresTempDir = config.wiresTempDir
tsharkLocation = config.tsharkLocation

#test chiru or local server (default if no parameter is given)
#js = scriptDir + "/" + "autoConnect.js"
js = scriptDir + "/" + "autoConnectLocal.js"

def main():
    operations(numberOfRuns)
    movePcapFiles()
    cleanup()
    autoreport.autoreport(testName)
    os._exit(1)

def operations(numberOfRuns):
    global avatarLogs
    global timeStamp

    #create a temp directory for pcap files
    if not os.path.exists(wiresTempDir):
        os.makedirs(wiresTempDir)

    #create a directory for log files
    timeStamp = time.strftime("%Y-%m-%dT%H:%M:%S%Z", time.localtime())
    avatarLogs = logsDir + "/logs_" + timeStamp
    os.makedirs(avatarLogs)
    #change working directory for running tundra
    os.chdir(rexbinDir)
    
    for i in range(1,numberOfRuns+1):
        run_clients(numberOfClients, i)
    os.chdir(scriptDir)

def run_clients(numberOfClients, i):
    for j in range(1,numberOfClients+1):
        #os.name options: 'posix', 'nt', 'os2', 'mac', 'ce' or 'riscos'
        if os.name == 'posix' or os.name == 'mac':
            #modify wireshark temp folder owner (required for tshark when capturing all devices)
            posixModTempfolder("root")
            p1 = Process(target=posixRunTshark, args=(i, j))
            p2 = Process(target=posixRunViewer, args=(i, j))
            #start tshark
            p1.start()
            print "writing network log to file captured" + str(i) + "." + str(j) + ".pcap"
            #start viewer
            p2.start()
            print "writing log to file naaliLog" + str(i) + "." + str(j) + ".log"
            running = True

        elif os.name == 'nt':	#NOT TESTED
            p1 = Process(target=ntRunTshark, args=(i, j))
            p2 = Process(target=ntRunViewer, args=(i, j))
            #start tshark
            p1.start()
            print "writing network log to file captured" + str(i) + "." + str(j) + ".pcap"
            #start viewer
            p2.start()
            print "writing log to file naaliLog" + str(i) + "." + str(j) + ".log"
            running = True

        else:
            print "os not supported"

    #while-loop to check if viewer is running
    while running == True:
        if not p2.is_alive():
            running = False
            posixModTempfolder("user")
            p1.terminate()
        else:
            time.sleep(1)

def posixModTempfolder(mode):
    if mode == "root":
        subprocess.call(['sudo','chown','root:root', wiresTempDir])
    elif mode == "user":
        subprocess.call(['sudo','chown','-R', getpass.getuser(), wiresTempDir])

def posixRunTshark(i,j):
    subprocess.call(['sudo','tshark','-i','any','-f','port 2345','-w', wiresTempDir + '/captured' + str(i) + '.' + str(j) + '.pcap'])

def posixRunViewer(i,j):
    x = "./Tundra --headless --run " + js + " 2>&1 | tee " + avatarLogs + "/naaliLog" + str(i) + "." + str(j) + ".log"
    subprocess.call(x, shell = True)

def ntRunTshark(i,j):
    subprocess.call([tsharkLocation + 'tshark.exe','-i','any','-f','port 2345','-w', wiresTempDir + '/captured' + str(i) + '.' + str(j) + '.pcap'])

def ntRunViewer(i,j):
    x = rexbinDir + "/Tundra.exe --config viewer.xml --headless --run " + js + " 2>&1 | tee " + avatarLogs + "/naaliLog" + str(i) + "." + str(j) + ".log"
    subprocess.call(x, shell = True)

def movePcapFiles():
    print "moving pcap files..."
    pcap = glob.glob(wiresTempDir + '/captured*.*.pcap')
    for i in range(0,len(pcap)):
        shutil.move(pcap[i], avatarLogs)
    print "pcap files moved to " + avatarLogs

def cleanup():
    print "cleaning up..."
    if os.path.exists(wiresTempDir):
        os.removedirs(wiresTempDir)

if __name__ == "__main__":
    parser = OptionParser()
    parser.add_option("-r", "--runs", type="int", dest="numberOfRuns")
    parser.add_option("-c", "--clients", type="int", dest="numberOfClients")
    parser.add_option("-j", "--js", dest="js")
    (options, args) = parser.parse_args()
    if options.numberOfRuns:
        numberOfRuns = options.numberOfRuns
    if options.numberOfClients:
        numberOfClients = options.numberOfClients
    if options.js == "local":
        js = scriptDir + "/autoConnectLocal.js"
    if options.js == "chiru":
        js = scriptDir + "/autoConnect.js"
    main()

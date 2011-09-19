#!/usr/local/bin/python

#import
import os
import sys
import time
import zipfile
import ftplib
import glob
import pexpect
from optparse import OptionParser
import config

# FTP-CONFIG for fileUpload
FTPHOST=config.launcherFTPHOST
FTPUSER=config.launcherFTPUSER
FTPPASSWD=config.launcherFTPPASSWD

#config
testlist = []
runlist = []
paramlist = []
exitstatus = []
pw="hardcoded" #default password if -p is not used
passwordSet=False
eoftimeout = 9999 #must be long enough for each test to complete
# suffixes to accept for testing
suffix = (".py", ".js")

# make a single archive file, if set to False file uploading is also disabled, 
# single archive uploads can be enabled in autoreport.py
archiveResults = config.launcherArchiveResults
uploadFile = config.launcherUploadFile

def main():
    setup()
    runTests(runlist, paramlist, numberOfTests)
    summary(runlist, numberOfTests, exitstatus)
    if archiveResults:
        zipAll()

def setup():
    global numberOfTests
    # list of tests, files with the suffixes configured above are tested if they exist
    # and checked for optional parameters
    testlist.append("js-viewer-server-test.py -f " + config.rexbinDir + "scenes/Avatar/avatar.txml")
    testlist.append("launchtundra.py -p '--server --headless --protocol udp --file " + config.rexbinDir + "scenes/TestScenes/PlaceableTest/placeabletest.txml'")
    
    #scripts that need to be run as super-user, 
    # if password is not set on launch these tests will not be added to the run queue
    if passwordSet:
        testlist.append("avatar-test.py -r 1 -c 1 -j local")
    else:
        print('No password given: sudo tests omitted from queue, use "-p password123" parameter')

    #for-loop to check if all testscript files exist
    for i in range(0,len(testlist)):
        for suff in range(0,len(suffix)):
            #split into 2 parts separated by suffix configured above
            list = testlist[i].split(suffix[suff], 2)
            # add the stripped suffix to file name
            file = list[0] + suffix[suff]
            # do not add invalid files
            if os.path.isfile(file):
                runlist.insert(i, file)
                #parameter found
                if len(list) > 1:
                    paramlist.insert(i, list[1])
                else:
                    paramlist.append(None)
    numberOfTests = len(runlist)

def runTests(runlist, paramlist, numberOfTests):
    for i in range(0,numberOfTests):
        try:
            if  paramlist == None:
                command = "python " + runlist[i]
            else:
                command = "python " + runlist[i] + str(paramlist[i])
        except:
            print("failed ")
        child = pexpect.spawn(command)
        child.logfile=sys.stdout
        i = child.expect(['password for', pexpect.EOF], timeout=eoftimeout)
        if i == 0: #password prompt found
            time.sleep(1)
            child.sendline(pw)
        child.expect(pexpect.EOF, timeout=eoftimeout)
        child.isalive()
        exitstatus.append(child.exitstatus)
        child.close()

def summary(runlist, numberOfTests, exitstatus):
    print "The following tests were in the run-queue:"
    for i in range(0,numberOfTests):    
        print "--" + runlist[i]
    print "The following tests were run succesfully:"
    for i in range(0,numberOfTests):
        if not exitstatus[i] == None:
            print "--" + runlist[i]

def zipAll():
    global timeStamp
    global zipName
    timeStamp = time.strftime("%Y-%m-%dT%H:%M:%S%Z", time.localtime())
    zipName = "testrun_" + timeStamp + ".zip"
    archives = glob.glob("*.zip")
    print "Creating testrun zip...",
    try:
        for i in range(0, len(archives)):
            if (("testrun_" in archives[i]) == False):
                z = zipfile.ZipFile(zipName, 'a',zipfile.ZIP_DEFLATED)
                z.write(archives[i])
                z.close()
                os.remove(archives[i])
        print ("succesful")
        if(uploadFile==True):
            fileUpload(zipName)
    except:
        print ("failed")
            
def fileUpload(zipName):
    # ftplib
    #comma to prevent newline
    print "Uploading zip... ",
    try:
        s = ftplib.FTP(FTPHOST, FTPUSER, FTPPASSWD) # server,login, passwd
        f = open(zipName,'rb') # file to send (read, binary)
        s.storbinary('STOR ' + zipName, f) # Send file
        f.close() # Close file and FTP
        s.quit()
        print("successful")
    except:
        print("failed ")

if __name__ == "__main__":
    parser = OptionParser()
    parser.add_option("-p", "--password", dest="pw")
    (options, args) = parser.parse_args()
    if options.pw:
        pw = options.pw
        passwordSet=True
    main()



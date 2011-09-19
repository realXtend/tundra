#!/usr/local/bin/python

#TODO: 
# - try catches
# - globals-
# - os.tmpfile() http://docs.python.org/library/os.html#os.tmpfile
# feature: archives all avatar-test output folders if moveold isnt enabled (or preservelogs is on) wildcard glob
#

import time
import StringIO
import ftplib
import glob
import os
import pycurl
import shutil
import zipfile
import os.path
import platform
import config


# FTP-CONFIG for fileUpload
FTPHOST=config.autoreportFTPHOST
FTPUSER=config.autoreportFTPUSER
FTPPASSWD=config.autoreportFTPPASSWD

# GITHUB-CONFIG for createGithubIssue
GITHUBLOGIN="rex-test-autoreport" # user
GITHUBAPITOKEN="4b05500b0593b50f4c89dacb6d11449c" #users api token
GITHUBREPO="rex-test-autoreport/issueReportTest" # e.g. "projectowner/project"

# OPTIONS
configPreserveLogs = config.preserveLogs
configMoveOld = config.moveOld
configCleanUp = config.cleanUp
configUploadFile = config.uploadFile
configCreateGithubIssue = config.createGithubIssue
includeMachineInfo = config.includeMachineInfo



# CONSTANTS
# configured test cases
TEST0 = "Example-skeleton"
TEST1 = "js-viewer-server-test"
TEST2 = "avatar-test"
TEST3 = "launchtundra"
# misc
tempCount = "count.txt"
tempErrors = "errors.txt"
#tempCount = tempfile.mktemp()
#tempErrors = tempfile.mktemp()

# VARIABLES
# bools
errors = False

# names
testName = None

# files
html = None
zipName = None
logFile = None
outputFile = None

# directories
logDir = None
oldDir = None

# misc
timeStamp = None
testComment = None
errorPattern = None


def whichTestWasRun(option):
    if option == TEST1:
        jsViewerServerTest()
    elif option == TEST2:
        avatarTest()
    elif option == TEST3:
        launchTundra()
    else:
        print("Error: test config not found")

def jsViewerServerTest():
    global testName
    global testComment
    global errorPattern
    global logDir
    global logFile
    global outputFile

    testName = TEST1
    testComment = "This test tries to launch local server and connect to it with the viewer app."
    logDir = "logs/jvst-output"
    errorPattern = [
        'Error',
        'fail'
    ]
    logFile = glob.glob(logDir + '/*.out')
    outputFile = glob.glob(logDir + '/*') #everything in outputDir
    operation()

def avatarTest():
    global testName
    global testComment
    global errorPattern
    global logDir
    global logFile
    global outputFile

    testName = TEST2
    testComment = "This test tries to connect to the server multiple times and move the avatar around, while recording traffic with tshark"
    logDir = "logs/avatar-output"
    errorPattern = [
        'Error',
        'fail'
    ]
    logFile = glob.glob(logDir + '/logs_*/naaliLog*.*.log')
    outputFile = glob.glob(logDir + '/*/*') #everything in outputDir
    operation()

def launchTundra():
    global testName
    global testComment
    global errorPattern
    global logDir
    global logFile
    global outputFile

    testName = TEST3
    testComment = "This test launches tundra2 with given parameters"
    logDir = "logs/launchtundra"
    errorPattern = [
        'FAIL: ',
        'Result: false'
    ]
    logFile = glob.glob(logDir + '/*.out')
    #files included in the zip archive
    outputFile = glob.glob(logDir + '/*') #everything in outputDir, script presumes test outputs everything to its own output folder, files can also be added to a list individually
    operation()

def operation():
    global html

    html = testName + "_summary.html"
    print("Running autoreport for: " + testName)

    whatWentWrong()

    if errors:
        createSummary()
        createArchive()
        if configUploadFile:
            uploadFile()
        if configCreateGithubIssue:
            createGithubIssue()      
    if not configPreserveLogs:
        #remove files
        for f in outputFile:
            os.remove(f)
        #remove dir
        #os.removedirs(logDir)
        shutil.rmtree(logDir, ignore_errors=True)
    else:
        #relocate already archieved files
        if configMoveOld:
            moveOld()
    if configCleanUp:
        cleanUp()
    print "Finished..."


def whatWentWrong():
    global errors
    errorAmounts = len(errorPattern)*[0]

    #reset temp file
    with open(tempErrors, 'w') as fo:
        fo.write("")
    #loop through files
    for i in range(0, len(logFile)):
        #make sure there is such a file
        if os.path.isfile(logFile[i]):
            #open file for reading
            with open(logFile[i]) as f:
                #open tempFile for appending
                with open(tempErrors, 'a') as fo:
                    #write file identifier
                    fo.write("---" + logFile[i] + "\n")
                #go through the file line by line
                for line in f:
                    #compare to all given indicators
                    for n in range(0, len(errorPattern)):
                        #if a matching is found print the line to tempFile and set vars for making report files
                        if  errorPattern[n] in line:
                            #set errors to true, for operation() knows to continue
                            errors = True
                            #count errors
                            #insert new value to n,get previous value with pop(which also removes it) and add one
                            errorAmounts.insert(n, errorAmounts.pop(n) + 1)
                            #write errors to file
                            with open(tempErrors, 'a') as fo:
                                fo.write(line)
        else:
            print "Configured logfile not found. Skipping..."

    if os.path.isfile(tempCount):
        os.remove(tempCount)
    #make count tempfile
    for i in range(0, len(errorPattern)):
        with open(tempCount, 'a') as cnt:
            cnt.write(str(errorPattern[i]) + " - " + str(errorAmounts[i]) + "\n")

def createSummary():
    global html

    print "Creating report...",
    try:
        htmlReportFile = open(html, 'w')

        summaryOperations(htmlReportFile, "start")
        if includeMachineInfo == True:
            summaryOperations(htmlReportFile, "machine")
        summaryOperations(htmlReportFile, "desc")
        summaryOperations(htmlReportFile, "found")
        summaryOperations(htmlReportFile, "errors")
        for i in range(0, len(outputFile)):
            summaryOperations(htmlReportFile, "add", outputFile[i])
        summaryOperations(htmlReportFile, "total")
        summaryOperations(htmlReportFile, "stop")

        htmlReportFile.close()
        print("successful")
    except:
        print("failed")

def summaryOperations(htmlReportFile, option, *fileName):
    errorFile = open(tempErrors, 'r')
    errorOutput = errorFile.read()
    errorFile.close()

    if option == 'start':
        htmlReportFile.write("<html><body><h1>TEST SUMMARY: " + testName + "</h1>")
    elif option =='machine':
        htmlReportFile.write("<p>Test machine info: " + platform.platform() + "<p>")
    elif option == 'desc':
        htmlReportFile.write("<p>" + testComment + "</p><hr />")
    elif option == 'found':
        htmlReportFile.write("<pre>")
        with open(tempCount,'r') as a:
            for line in a:
                htmlReportFile.write(line)
        htmlReportFile.write("</pre><hr />")
    elif option == 'errors':
        htmlReportFile.write("<pre>" + errorOutput + "</pre><hr /><h3>Files:</h3><ul>")
    elif option == 'add':
        htmlReportFile.write("<li>File: <a href=" + str(fileName)[2:-3] +">" + str(fileName)[2:-3] + "</a></li>") # [2:-3] strip list brackets ('xxx',) --> xxx
    elif option == 'total':
        htmlReportFile.write("</ul><hr /><p>Total files: " + str(len(outputFile)) + "</p>")
    elif option == 'stop':
        htmlReportFile.write("</body></html>")

def createArchive():
    global timeStamp
    global zipName

    print "Creating zip... ",
    try:
        timeStamp = time.strftime("%Y-%m-%dT%H:%M:%S%Z", time.localtime())
        zipName = testName + "_" + timeStamp + ".zip"

        for i in range(0, len(outputFile)):
            if os.path.isfile(outputFile[i]):
                zipFiles(outputFile[i])
            else:
                print "Configured file missing. Skipping over..."

        if os.path.isfile(html):
            zipFiles(html)
        else:
            print "Report file missing !"
        print("successful")
    except:
        print("failed")

def zipFiles(file):
    #open file for appending
    z = zipfile.ZipFile(zipName, 'a',zipfile.ZIP_DEFLATED)

    #add created files to a zip archive
    z.write(file)
    z.close()

def moveOld():
    # os.shutil
    global oldDir

    oldDir = "old/" + testName + "_" + timeStamp

    if not os.path.exists(oldDir):
        os.makedirs(oldDir)
    for i in range(0,len(outputFile)):
        shutil.move(outputFile[i], oldDir)

def uploadFile():
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
        global uploadFailed
        uploadFailed = True

def createGithubIssue():
    githubUrl="https://github.com/"
    print "Creating issue: " + githubUrl + GITHUBREPO + "/issues"

    url = "http://github.com/api/v2/json/issues/open/" + GITHUBREPO

    errorZipLink = "ftp://" + str(FTPHOST) + "/" + zipName

    issueTitle="Auto-reported: " + testName
    issueBodyInfo="""
Errors were found during a test.
This issue was created automatically by the auto-report script.
"""
    ziplink = ""
    # don't print link if upload is disabled or failed to configured host
    if configUploadFile == True and uploadFailed == False:
        ziplink = "\n\nError logs for this report can be found in here: " + errorZipLink
    # notify reader about upload failing
    elif uploadFailed == True:
        ziplink = "\n\nUpload failed to ftp host: " + FTPHOST
    with open(tempCount, 'r') as cnt:
        countPrint = cnt.read()
    issueBody = issueBodyInfo + "\n\nFound - count:\n" + countPrint +  ziplink


    a1 = "login="+GITHUBLOGIN
    a2 = "token="+GITHUBAPITOKEN
    a3 = "title="+issueTitle
    a4 = "body="+issueBody

    c = pycurl.Curl()
    c.setopt(c.URL, url)
    #buffer for redirecting curl output
    output = StringIO.StringIO()
    c.setopt(pycurl.WRITEFUNCTION, output.write)
    c.setopt(c.POSTFIELDS, a1 + '&' + a2 + '&' + a3 + '&' + a4)
    c.perform()

def cleanUp():
    print "Cleaning up...",
    try:
        # remove tempfiles
        if os.path.isfile(tempErrors):
            os.remove(tempErrors)
        if os.path.isfile(tempCount):
            os.remove(tempCount)
        if os.path.isfile(html):
            os.remove(html)
        print("successful")
    except:
        print("failed")

def autoreport(test):
    whichTestWasRun(test)
    
if __name__ == "__main__":
    #test main for standalone run
    tmp = "js-viewer-server-test"
    autoreport(tmp)

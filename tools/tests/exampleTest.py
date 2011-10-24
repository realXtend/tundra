####### ENABLE AUTO REPORTING IN A TEST
# arrows point to the additions needed for autoreporting

import autoreport #<--<<

# This is an example test script
print "example test script"
#test goes here

#send the config name to autoreport and create the report if needed
autoreport.autoreport("exampletest") #<--<<

#################### ADD THE TEST CONFIG TO AUTOREPORT
# Additions needed in autoreport.py
# under CONSTANTS at around line 43, add
"""
TEST4 = "exampletest"
"""
# to function whichTestWasRun(option), add
"""
elif option == TEST4:
    launchTundra()
"""


# add setup function
"""
 def exampleTest():
    global testName
    global testComment
    global errorPattern
    global logDir
    global logFile
    global outputFile

    testName = TEST4
    testComment = "This is an example"
    logDir = "logs/exampletest"
    # words to look for in the logfiles
    errorPattern = [ 
        'failure',
        'error'
    ]
    # files 'grepped' for the error patters
    logFile = [
        '1.example',
        '2.example',
        '3.example'
    ]
    #files included in the zip archive
    outputFile = glob.glob(logDir + '/*') #everything in outputDir, script presumes test outputs everything to its own output folder, files can also be added to a list individually
    operation()
"""

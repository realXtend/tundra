#!/usr/local/bin/python
import getpass
import os
import os.path

#COMMON
rexbinDir = "/home/" + getpass.getuser() + "/src/realxtend/naali/bin/" # make sure this refers to the tundra bin/

# wireshark directory for windows
tsharkLocation = "c:/Program Files/Wireshark/"

# FILE: LAUNCHER
# make testrun_*.zip
launcherArchiveResults = True

# uploade archive
launcherUploadFile = False

# host where testrun_*.zip should be uploaded
launcherFTPHOST="xxx"
launcherFTPUSER="xxx"
launcherFTPPASSWD="xxx"

# FILE: AUTOREPORT
# Option for uploading report .zip to host
# default false (needs config)
uploadFile = False

# host where single test zip archives should be uploaded
autoreportFTPHOST="xxx"
autoreportFTPUSER="xxx"
autoreportFTPPASSWD="xxx"

# Option for making github issue report
# default false (needs config)
createGithubIssue = False

# Option for preserving test output files after archive has been created
preserveLogs = False

# Option for moving test output files to an old archives folder (debug-standalone-run)
# if preserveLogs is set to false option is ignored
moveOld = False

# Option for cleaning up (removing temp files) the the folder after running the script
# default true
cleanUp = True

#Option for including test machine info to the report file
includeMachineInfo = True


################# THINGS BELOW THIS LINE SHOULD NOT NEED EDITING
### FOLDER CONFIG ###
scriptDir = os.path.abspath(os.getcwd())
testDir = os.path.abspath(os.path.join(scriptDir, '..'))

# FILE: JVST-TEST
jvstLogsDir = os.path.abspath(os.path.join(scriptDir, 'logs/jvst-output/'))

# FILE: AVATAR-TEST
avatarLogsDir = os.path.abspath(os.path.join(scriptDir, 'logs/avatar-output'))
wiresTempDir = os.path.abspath(os.path.join(scriptDir, 'wireshark_temp'))

# FILE: LAUNCHTUNDRA-TEST
tundraLogsDir = os.path.abspath(os.path.join(scriptDir, 'logs/launchtundra/'))

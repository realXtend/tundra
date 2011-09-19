--------------------------------------
README for Tundra2 python test scripts
--------------------------------------

-----------------------------------------
1. File descriptions & usage instructions
-----------------------------------------

- autoreport.py
    - creates a zip containing the html summary + log files for each test

- config.py
    - define launcher + auto-report settings & test-related directories here

- launcher.py 
    - runs all configured tests: (needs root password if avatar test is included because of tshark)
    - parameters:
        -p, --password <password> (default "hardcoded")
    - usage example: 
        python launcher.py -p passwd123

- js-viewer-server-test.py
    - tries to launch local server and connect to it with the viewer app
    - parameters:
        -f, --file <path/to/file> (default ~/realxtend/bin/scenes/Avatar/avatar.txml)
    - usage example: 
        python js-viewer-server-test.py -f ~/realxtend/bin/scenes/Avatar/avatar.txml

- avatar-test.py
    - tries to connect to the server multiple times and move the avatar around, while recording traffic with tshark
    - parameters:
        -r, --runs 	number of runs (default 2)
        -c, --clients 	number of clients per run (default 2)
        -j, --js <chiru/local> 	select server (default local)
    - usage example: 
        python avatar-test.py -r 1 -c 1 -j chiru

- launchtundra.py
    - launches tundra server/viewer with given scene/script etc. parameters 
    - parameters:
        -p, --parameters   run configuration parameters for tundra2
    - usage example:
        python launchtundra.py -p '--server --protocol udp --file scenes/scenex/x.txml'

-------------------------
2. How to add a new test?
-------------------------

- In config.py, define the logs folder(s) path for your test under "FOLDER CONFIG"

- Now you can import config to your newtest.py and add all the needed folders as follows: logsDir = config.testXLogsDir

- In autoreport.py
    - Add definition "TESTX = "newtest" under "CONSTANTS"
    - Add a new function def newTest(), use the other functions for example def avatarTest() as a reference
    - Add new elif condition for your test in whichTestWasRun function: elif option == TESTX: newTest()

- Add variable "testName" to your newtest.py and import autoreport
- Add a call to function autoreport.autoreport(testName). It needs to be called after test functions are completed

- Finally, you need to modify launcher.py
    - If your test doesn't require sudo password, add testlist.append("newtest.py -parameters") to the setup function
    - If your test requires sudo password, add testlist.append("newtest.py -parameters") under the "if passwordSet:"
      condition

- If you encounter problems, check exampleTest.py for some additional information


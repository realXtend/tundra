realXtend Naali
===============

System requirements
-------------------
-GPU with Shader Model 3.0 support
-(Windows only) Updated DirectX End-User Runtime from http://www.microsoft.com/directx


Getting Started
---------------
After a succesful initialization, you will see the login window and the OGRE rendering window behind it. Naali provides three different connection modes: OpenSim, realXtend authentication and OpenID.

-Opensim
Use the OpenSim connection method to connect to an OpenSim server with or without authentication, or to a legacy realXtend server (version 0.4 and above) without authentication. If the server you are going to connect doesn't use authentication, no password is required. Just insert some username ([first name] [lastname]), server address and port ([address]:[port]) and click connect. 
If you are connecting to Taiga servers via OpenSim authentication, you must give proper firstname, lastname and password, and the sim must have a corresponding account. Also the default world port is 8002 and NOT the usual 9000 because you are authenticating to a cablebeach worldserver, not directly to the sim. 

-reX authentication
realXtend authentication connection method can be used to login to a legacy realXtend server and Taiga server suite with authentication. Insert your username, password, server and authentication server adresses and ports, and click connect. 

Note: The viewer will use 9000 and 10001 as the default ports for the servers if you don't specify them yourself.

-OpenID
You can use OpenID login to connect to realXtend Taiga server. For this connection method, you have to have a valid Taiga OpenID account. In the login window, select OpenID tab and click Connect. Set your world and openid urls, for example: World: 'http://taiga-server-address:8002/login' and OpenID: 'http://taiga-server-address:8004/YourAccountName'. OpenID provider can be located in a different server than the world server. You need only one Taiga OpenID account in order to be able to connect on any Taiga server.

NOTE: You can create a Taiga OpenID account by going to http://taiga-server-address:8004/signup with your browser and filling out the forms. Default ports for Taiga are 8002 for World server and 8004 for the OpenID server. You need to include the ports to the urls in the OpenID login window fields. Also remember that you need to include "/login" in the url path on world server field and "/YourUserName" in the OpenID field.

After a succesful connection the login window disappears and logout and quit buttons appear to the bottom right corner of the screen. Use "Logout" button to log out and get back to the login window and "Quit" button to terminate the program.


Controls
--------
W, A, S, D or ARROWS   Navigate in the world
F                      Toggle the fly mode
SPACE                  Jump / fly upwards in fly mode
C                      Crouch (doesn't work in ModRex server) / fly downwards in the fly mode
MOUSE WHEEL            Move the camera closer / further off the avatar
CTRL+TAB               Toggle the free-look mode
LEFT MOUSE             Touch object
RIGHT MOUSE            Rotate avatar, or look around in mouse-look mode (see below)
F1                     Toggle debug console

You can enter the mouse-look mode by zooming "into" the avatar with mouse wheel. In mouse-look and free-look modes, hold down the right mouse button to look around and, if you are moving simultaneously, to steer the avatar.


Avatar editor
-------------

You can load avatars or attachments from any directory by pressing the New Avatar & New Attach buttons, but you should have all the files needed by a single avatar or attachment (.xml, .mesh, .skeleton, .material and texture files) in the same directory.

Note that currently saving the avatar (and thus making it visible to others) works only when used with the legacy reX authentication & reX avatarstorage servers. 


Python
------
It is possible to run arbitary Python code within the viewer. A way is to put the code to bin/pymodules/usr/command.py and then the alt & period key (alt-.) in the viewer window. See bin/pymodules/readme.txt for info about how to make own modules, bind arbitary keys etc. F11 is used to reload the code on those Python written modules, so that you don't have to restart the whole viewer when change the code or UI there.

IM communication
----------------
Naali viewer supports jabber protocol for IM communication. You can use any jabber provider or Google Talk accounts.


Compiling
---------
CMake is used to create the build environment.
See doc/cmake.txt for more information.


Dependencies
------------
See doc/dependencies.txt for more detailed instructions on dependencies.


Windows
-------
read doc/build-windows.txt

Quickstart for Windows / Visual Studio: install CMake, then run cmake.bat if not using VS2008, and cmake_vs2008.bat, if using VS2008.

N.B. When compiling for the first time, compile in debug mode and run the exe at least once.
---- Some data files are created in debug mode which are needed when running in release mode.


Linux
-----
read doc/build-linux.txt


License
-------
See license.txt file for license information.


Unit Testing
------------
Boost Unit Test Framework is used for unit testing. See boost documentation for more information.
The test runner is called UnitTests.exe and it is located in the same directory as the application executable.
For return values, refer to boost documentation.

Always run unit tests in debug mode, since some tests rely on asserts.
Also run tests in final release mode.


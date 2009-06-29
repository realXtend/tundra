realXtend Naali 0.0.1
=====================

System requirements
-------------------
-GPU with Shader Model 3.0 support


Getting Started
---------------
After a succesful initialization, you will see the login window and the OGRE rendering window
behind it. Naali provides three different connection modes: OpenSim, realXtend authentication and OpenID:

-Opensim
Use the OpenSim connection method to connect to an OpenSim server with or without authentication,
a legacy realXtend server (version 0.4 and above) without authentication. If the server you are going to
connect doesn't use authentication, no password is required. Just insert some username ([first name] [lastname]),
server address and port ([address]:[port]) and click connect. If you are connecting to Taiga servers via 
OpenSim authentication, you must give proper firstname, lastname and password, and the sim must have a corresponding
account.

-reX authentication
realXtend authentication connection method can be used to login to a legacy realXtend server and Taiga server
suite with authentication. Insert your username, password, server and authentication server adresses and ports, and click
connect. 

Note: The viewer will use 9000 and 10001 as the default ports for the servers if you don't spesify them yourself.

-OpenID
You can use OpenID login to connect to realXtend Taiga server. For this connection method, you have to have a valid Taiga
OpenID account. In the login window, select OpenID tab and click Connect. Set your world and openid urls, for example:
World: 'http://taiga-server-address:8002/login' and OpenID: 'http://taiga-server-address:8004/YourAccountName'.
OpenID provider can be located in a different server than the world server. You need only one Taiga OpenID account in order
to be able to connect on any Taiga server.

NOTE: You can create a Taiga OpenID account by going to http://taiga-server-address:8004/signup
with your browser and fillling out the forms. Default ports for Taiga are 8002 for World server and 8004 for the OpenID server.
You need to include the ports to the urls in the OpenID login window fields. Also remember that you need to include "/login"
in the url path on world server field and "/YourUserName" in the OpenID field.

After a succesful connection the login window disappears and logout and quit buttons appear to the bottom
right corner of the screen. Use "Logout" button to log out and get back to the login window and "Quit" button
to terminate the program.


Controls
--------
Navigate in the world by using W, A, S and D buttons. F toggles the fly mode. You can jump by pressing SPACE 
(flys upwards in fly mode) and crouch (doesn't work in ModRex) by pressing C (flys downwards in the fly mode).
Use the mouse wheel to move the camera closer to/further off the avatar. By pushing TAB you can enter to the
free-look mode. You can enter the mouse-look mode by zooming "into" the avatar. In mouse-look and free-look
modes, hold down the right mouse button to look around and, if you are moving simultaneously, to steer the avatar.


Console
-------
Toggle the console overlay by pressing the button above TAB: apostrophe (´) on US keyboard layout and section sign
(§) on scandic layout. Write "help" to get the summary of the available console commands.


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


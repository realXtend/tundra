realXtend Tundra
===============

Introduction
------------
Tundra is a scriptable 3D internet application development platform. It is aimed primarily for application developers, as a platform for creating networked 3D worlds with customized content.

Tundra is based on Qt and Ogre3D.

System requirements
-------------------
-GPU with Shader Model 3.0 support
-(Windows only) Updated DirectX End-User Runtime from http://www.microsoft.com/directx

Getting Started
---------------
Tundra uses a traditional server-client architecture for networking. After installing, you can find two executables, server.exe and viewer.exe in the Tundra start menu.

The Tundra Server is used for standalone-mode editing and viewing Tundra documents. To host a 3D scene, run Tundra in dedicated mode using the --headless command line option. See http://www.realxtend.org/doxygen/runningnaali.html for details.

The Tundra Viewer is the client software that is used to connect to a server.        

Compiling from the source
-------------------------
Tundra source code is available at https://github.com/realXtend , under the Apache 2.0 license. Remember to git checkout to the branch Tundra after cloning the repository.

git clone http://github.com/realXtend/naali.git
git checkout -b tundra origin/tundra
 
Tundra uses CMake as its build system.

Documentation
-------------
More information about Tundra can be found online at http://www.realxtend.org/doxygen/ . You can also generate the same documentation locally from doc/viewer.Doxyfile by using doxygen.

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

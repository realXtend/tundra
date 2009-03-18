realXtend Viewer
================

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


RexNG Viewer
============

Compiling
---------
CMake is used to create the build environment.
See documentation/cmake.txt for more information.

Quickstart for Windows / Visual Studio: install CMake, then run cmake.bat if not using VS2008, and cmake_vs2008.bat, if using VS2008.


Dependencies
------------
See external_libs/RexDependencies.txt for more detailed instructions on dependencies.


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


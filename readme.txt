RexNG Viewer
============

Compiling
---------
Solutions are provided for Visual Studio 8 and 9. Install dependencies and make sure Visual Studio can find them. Dependencies can also be installed to external_libs directory.


Dependencies
------------
Requires Boost and PoCo C++ libraries. See documentation/external libraries.txt for more detailed instructions on dependencies.

Boost:
http://www.boost.org/

PoCo:
http://pocoproject.org/


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


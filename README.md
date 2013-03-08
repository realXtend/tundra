realXtend Tundra
================

Tundra is a scriptable 3D internet application development platform. It is aimed primarily for application developers, as a platform for creating networked 3D worlds with customized content.

Tundra is licensed under [Apache 2.0] and based on [Qt] and [Ogre3D].

Getting Started
---------------

Tundra uses the traditional client-server architecture for networking. After installing you will find the `Tundra` (and `TundraConsole` on Windows) executable from the install directory, run `Tundra --help` for available command line parameters.
This executable can be configured to run a set of C++ and JavaScript plugins. You can create your own configuration file, or use the ones provided. Some examples:  
`Tundra --config viewer.xml` - Starts Tundra with a client configuration which provides an user interface for connecting to Tundra servers.  
`Tundra --connect localhost:2345;udp;TestUser` - Starts Tundra and automatically connects to a localhost server using the default server port and protocol.  
`Tundra --server --headless --port 6565 --protocol tcp` - Starts Tundra with the default plugin set in server mode serving TCP connections at port 6565. The Tundra server defaults are port 2345 and UDP protocol, for it you can simply run `Tundra --server --headless`. If no `--config` parameter is provided, the default plugins.xml is used.  

The Tundra server mode is used for standalone-mode editing and viewing Tundra documents. To host a 3D scene, run Tundra in dedicated mode using the `--server` and `--headless` command line parameters. The Tundra client mode is used to connect to a server.

See the `scenes` folder for example demo scenes and applications. F.e.x. `Tundra --file scenes/Avatar/scene.txml`

Compiling from Sources
----------------------

Tundra source code is available at the [realXtend github repository]. This repository hosts various branches for current and deprecated developmnet lines from the realXtend team, so be sure to checkout `tundra2` branch after cloning.

Tundra uses [CMake] as its build system and depends on various other open source projects. See more from `doc/dependencies.txt`.

### Windows

Visual Studio 2008 and 2010 build environments are currently supported. However, be advised that some of the optional dependencies (f.ex. VLC) are not usable with the newer Visual Studio versions. Make sure that you have the latest Visual Studio Service Packs installed.

 There are two ways of acquiring the dependencies **1)** Automated script to build them from sources **2)** Use prebuilt dependencies from an SVN repository.

**1) Building with Full Dependencies**

1. `cd tools` and run `BuildDepsVSXXX.cmd`, depending on your choice of Visual Studio version. The build script needs to be run in the Visual Studio Command Prompt for build tools and have several other utilities in your PATH. This script will print information what you need to proceed, follow the instructions carefully. You can abort the script with Ctrl+C at this point and setup your environment.
2. Once you are done setting up your build environment hit any key to continue the script as it instructs. Full depedency build will take about 2-3 hours.
3. After the script completes dependencies can be found from /deps (for VS2008), or deps-vsXXX (for newer VS versions). The needed runtime libraries are automatically copied to /bin.

Now run `windows-build-tundra.cmd`. This script will setup the needed build environment variables for CMake. Next it will run CMake to generate a tundra.sln solution file and build it.

If you want the script to build Tundra you need to run it in the Visual Studio Command Prompt as it needs MSBuild. However you can hit Ctrl+C after the CMake step finishes and open the solution file with the Visual Studio IDE, if that is what you prefer.

**2) Building with Prebuilt Dependencies**

Prebuilt dependencies are only available for Visual Studio 2008 at the moment.

1. `cd tools` and run `windows-fetch-prebuilt-deps.cmd`.
2. This will download the prebuilt dependency package, extract it to /deps-prebuilt, copies the needed runtime libraries to /bin and runs CMake against the prebuilt dependencies.
3. `cd ..`, open up tundra.sln and build.

### Linux

See [tools] for distro-specific build scripts.

### Mac OS X

See [tools/build-mac-deps.bash] for automated dependency and Tundra build script.

Developer Documentation
-----------------------

Tundra uses [Doxygen] as its main documentation tool. In order to generate and view the developer documentation, follow these steps:
1. have Doxygen installed,  
2. `cd doc`,  
3. `doxygen tundra.Doxyfile`,  
4. `cd html`, and  
5. open up `index.html`.

Contributing
------------
0. Preferably interact with the developers already in advance before starting your work.
1. Fork Tundra.
2. Preferably create a feature branch for your work.
3. Make sure to follow the coding conventions (doc/dox/CodingConventions.dox).
4. Make a pull request

Contact Information
-------------------

You can find Tundra developers from IRC `#realxtend-dev @ freenode`. Also check out the [user-oriented mailing list](http://groups.google.com/group/realxtend) and the [developer-oriented mailing list](http://groups.google.com/group/realxtend-dev).

Releases
--------

New releases are announced on the mailing lists and at the [realXtend blog]. The releases are uploaded to Google Code project site, that is used for hosting downloads. http://code.google.com/p/realxtend-naali/downloads/list

[Qt]:          http://qt.nokia.com/                            "Qt homepage"
[Ogre3D]:      http://www.ogre3d.org/                          "Ogre3D homepage"
[Apache 2.0]:  http://www.apache.org/licenses/LICENSE-2.0.txt  "Apache 2.0 license"
[CMake]:       http://www.cmake.org/                           "CMake homepage"
[realXtend blog]: http://www.realxtend.org                     "realXtend blog"
[realXtend github repository]: https://github.com/realXtend/naali/tree/tundra2 "realXtend Tundra repository"
[tools/build-mac-deps.bash]: https://github.com/realXtend/naali/blob/tundra2/tools/build-mac-deps.bash "tools/build-mac-deps.bash"
[tools]: https://github.com/realXtend/naali/tree/tundra2/tools "tools"
[Doxygen]:  http://www.stack.nl/~dimitri/doxygen/ "doxygen homepage"


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

Tundra source code is available at the [realXtend github repository]. This repository hosts various branches for current and deprecated development lines from the realXtend team, so be sure to checkout `tundra2` branch after cloning.

Tundra uses [CMake] as its build system and depends on various other open source projects. See more from `doc/dependencies.txt`.

### Windows

Visual Studio 2008 and 2010 build environments are currently supported. Make sure that you have the latest Visual Studio Service Packs installed.

The Tundra dependencies are acquired and built using an automated build script:  
1. `cd tools\Windows\VS<VersionNumber>\`  
2. Run `BuildDeps_<BuildType>`, or `BuildDepsX64_<BuildType>` (if wanting to do a 64-bit build). RelWithDebInfo is recommended for the common development work.  
   Note that the build script needs to be executed in the Visual Studio (x64 Win64) Command Prompt for having the required build tools and several other utilities in your PATH.  
   The build script will print information what you need in order to proceed, follow the instructions carefully. You can abort the script with Ctrl+C at this point and setup your environment.  
3. Once you are done setting up your build environment, hit any key to continue the script as it instructs. The full depedency build might take up to 2 hours.
4. After the script has completed, the dependencies can be found from either `deps\` (for 32-bit VS2008 build), or `deps-vs<VersionNumber>-<TargetArchitecture>\` (for other build configuration variants). The needed runtime libraries are automatically copied to `bin\`.  
5. Now run CMake batch script corresponding to your desired build configration. This script will set up the needed build environment variables for CMake and invoke CMake to generate a tundra.sln solution.  
6. Build Tundra using the solution file.

### Linux

Currently only Ubuntu, Mint, Debian, and Kali are officially supported. See [tools/Linux/build-deps.bash] for the unified build script. 

### OS X

See [tools/OSX/BuildDeps.bash] for automated dependency and Tundra build script.

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
4. Make a pull request.

Contact Information
-------------------

You can find Tundra developers from IRC `#realxtend-dev @ freenode`. Also check out the [user-oriented mailing list](http://groups.google.com/group/realxtend) and the [developer-oriented mailing list](http://groups.google.com/group/realxtend-dev).

Releases
--------

New releases are announced on the mailing lists and at the [realXtend blog]. The releases are available at the [realXtend Tundra Google Code] project site.

[Qt]:          http://qt.nokia.com/                            "Qt homepage"
[Ogre3D]:      http://www.ogre3d.org/                          "Ogre3D homepage"
[Apache 2.0]:  http://www.apache.org/licenses/LICENSE-2.0.txt  "Apache 2.0 license"
[CMake]:       http://www.cmake.org/                           "CMake homepage"
[realXtend blog]: http://www.realxtend.org                     "realXtend blog"
[realXtend github repository]: https://github.com/realXtend/naali/tree/tundra2 "realXtend Tundra repository"
[tools/build-mac-deps.bash]: https://github.com/realXtend/naali/blob/tundra2/tools/build-mac-deps.bash "tools/build-mac-deps.bash"
[tools/Ubuntu/build-deps.bash]: https://github.com/realXtend/naali/tree/tundra2/tools/Ubuntu/build-deps.bash "tools/Ubuntu/build-deps.bash"
[Doxygen]:  http://www.stack.nl/~dimitri/doxygen/ "doxygen homepage"
[realXtend Tundra Google Code]: http://code.google.com/p/realxtend-naali/downloads/list

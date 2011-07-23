About this 'pymodules' directory
================================

Here are the Python written modules that are run within the embedded interpreter in the viewer.

The directory layout is as follows:

core/ - internals (module & event manager), no need for users to touch
usr/ - place for own modules, local things that can extend the viewer
communication/ - telepathy using messaging, backend for CommunicationModule
apitest/ - unit test like viewer api usage tests, work as examples too
*/ - individual modules (python packages) which implement some feature. some enabled by default, some not.

To make your module to be loaded at viewer start, add it to default.ini or make a similar my.ini .
For it to be able to receive events from the viewer (now: keyboard presses, mouse input data, incoming in-world chat messages) it has to be be a component using the circuits library.

This system is now somewhat mature, and is used to implement some of the default ('core') features in the viewer, like mediaurl handling and 3d manipulation of objects. It has been also used in 3rd party project for custom UI tools etc. However, the API is still under heavy development and some of the first early hacks made here will be cleaned up during September-October 2010 or so. This will change those old parts of the API but adopting to the new ways should be simple.

Please be on touch on e.g. the realxtend-dev mailing list or on irc (#realxtend-dev on freenode) if have questions or proposals, like wishes about what to add to the API (we have some definite use cases but are interested to hear about others, see the wiki for design and plans).

About this 'pymodules' directory
================================

Here are the Python written modules that are run within the embedded interpreter in the viewer.

The directory layout is as follows:

core/ - internals (module & event manager), no need for users to touch
usr/ - place for own modules, local things that can extend the viewer
communication/ - telepathy using messaging, backend for CommunicationModule
apitest/ - unit test like viewer api usage tests, work as examples too

To make your module to be loaded at viewer start, add it to autoload.py .
For it to be able to receive events from the viewer (now: keyboard presses, mouse input data, incoming in-world chat messages) it has to be be a component using the circuits library.

This is very early experimental work, and there is not much in the API yet. Please be on touch on e.g. the realxtend-dev mailing list or on irc (#realxtend-dev on freenode) if have questions or proposals, like wishes about what to add to the API (we have some definite use cases but are interested to hear about others, see the wiki for design and plans).

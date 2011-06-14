This test scene implements movement control features that allow easily location and orientation of entities,
as well as the definition of paths to be followed.

To test it, run the server loading the scene and start one or more clients to connect to it.

Each client should get an avatar that can be controlled with WASD + arrows + mouse. F toggles fly mode, 
space flies up and C flies down. Space jumps and C sits down when you are not in fly mode.
Q makes a wave gesture. Mouse scroll and +/- zooms in/out. R toggles running when you are not flying.
Ctrl + left mouse click makes the avatar go to the selected point.

Also note that collisions are kept at any time.

There must also be a non player character in the scene following a predefined path. 
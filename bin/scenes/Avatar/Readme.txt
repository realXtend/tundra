This test scene implements avatars. It uses two script files: jsmodules/avatar/avatarapplication.js, which hooks
to connects/disconnects of users and creates/deletes avatars for them, as well as handles the client's camera switching
(ctrl+tab), and jsmodules/avatar/simpleavatar.js, which implements movement & animation of a single avatar.

To test, copy default_avatar.xml, fish.mesh & WoodPallet.mesh to your tundra bin/data/assets directory, and avatar.xml
to your bin directory.

Then, run the server and load the scene on it by drag-and-dropping avatar.txml to the main window, or by using console
command loadscene(avatar.txml)

Next, start one or more clients and connect to the server. Each client should get an avatar that can be controlled
with WASD + arrows + mouse. F toggles fly mode, space flies up and C flies down. Space jumps and C sits down when you are not in fly mode.
Q makes a wave gesture. Mouse scroll and +/- zooms in/out.

Only third person camera is currently implemented. Also note that collisions are disabled when flying for now.

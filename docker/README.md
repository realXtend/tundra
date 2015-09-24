# How to use this Dockerfile

This Dockerfile can be used to build an image of the FIWARE Synchronization GE server, aka realXtend Tundra.

It is based on Ubuntu 14.04. Building requires a 64bit .deb image of Tundra, which can be created using the
tools/Linux/package-ubuntu-tundra.bash script.

The image will expose ports 2345 (real-time communications default port) and 2346 (SceneAPI REST
protocol default port)

To build the image:

    docker build -t synchronization

To run the image in interactive mode (shell), with the container's ports exposed:

    docker run  -P -it synchronization

Running Tundra inside the image requires using xvfb-run and headless mode, for example:

    cd /opt/realxtend-tundra
    xvfb-run ./Tundra --file scenes/Physics2/scene.txml --server --headless --httpport 2346

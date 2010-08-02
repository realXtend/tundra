To build Kristalliserver and the Naali modules that enable Kristalliprotocol 
support, you must first build the support library Kristalli, by opening 
Kristalli/build/Win/clbLib.sln and building the Debug & Release targets. 
Currently only Visual Studio 2008 is supported.

(Additionally, if you build Naali with RelWithDebInfo, you must manually copy
Kristalli/build/Win/Release directory into Kristalli/build/Win/RelWithDebInfo)

Afterwards you should be able to build Naali normally (the Kristalli modules
have been added into root cmakelists as mandatory).

Build the Kristalliserver by opening KristalliServer/Kristalli.sln and building
the targets you like.

When logging into a world, Naali will now attempt to connect to the
KristalliServer in the same IP as the simulator, using UDP port 2345. So you
should run KristalliServer on the same machine as the sim, using the commandline

kristalliserver udp 2345

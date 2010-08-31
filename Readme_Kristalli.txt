To build the Naali modules that enable Kristalliprotocol support, you must 
first build the support library Kristalli, by opening Kristalli/build/Win/
clbLib.sln and building the Debug & Release targets. Currently only Visual 
Studio 2008 is supported.

(Additionally, if you build Naali with RelWithDebInfo, you must manually copy
Kristalli/build/Win/Release directory into Kristalli/build/Win/RelWithDebInfo)

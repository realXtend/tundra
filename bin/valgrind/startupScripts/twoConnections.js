//FileType: RealXtend naali Valgrind test-script
//Function: Make two connections both to different local servers and disconnect/exit the program.
//Date: 10.08.2011
//Made by: Jukka Vatjus-Anttila / Center for Internet Excellence

var doFirst = true;
// Start this connection process when FrameAPI sends first frameUpdate signal
frame.Updated.connect(StartProcess);

function StartProcess()
{
    // This function starts the whole process. Only run it at 1st frame and then set doFirst to false.
    if (doFirst)
    {
        doFirst = false;          
        // Wait 40 seconds before doind any connection attempts because valgrind startup is so long. 
        frame.DelayedExecute(40).Triggered.connect(this,AutoConnect1);
    }
}
function Disconnect1()
{
    // Disconnect second connection we made.
    console.ExecuteCommand("Disconnect(1)");
    // Again we wait for valgrind.
    frame.DelayedExecute(150).Triggered.connect(this,Disconnect2);
}

function Disconnect2()
{
    // Just exit the program which closes the last connection.
    console.ExecuteCommand("Exit");
}    
function AutoConnect1()
{
    var isserver = server.IsRunning() || server.IsAboutToStart();
    if (!isserver)
    {
        // Make 1st connection to one of the local servers
        client.Login("localhost",2345,"Guest","","tcp");
        // Again wait for valgrind. We dont want to make next connection before 1st connection handshake is even completed.
        frame.DelayedExecute(270).Triggered.connect(this,AutoConnect2);
    }     
}
function AutoConnect2()
{
    // Make 2nd connection to one of the local servers
    client.Login("localhost",2346,"Guest","","tcp");
    // Again we wait for valgrind.
    frame.DelayedExecute(270).Triggered.connect(this,Disconnect1)
}
    


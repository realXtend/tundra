//FileType: RealXtend naali Valgrind test-script
//Function: Make one connection to localhost server, disconnect and make connection to another localhost server. Repeat few times.
//Date: 10.08.2011
//Author: Jukka Vatjus-Anttila / Center for Internet Excellence

var counter = 6;
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
        frame.DelayedExecute(20).Triggered.connect(this,ConnectionHandler);
    }
}  
function ConnectionHandler()
{
    var isserver = server.IsRunning() || server.IsAboutToStart();
    if (!isserver)
    {
        frame.DelayedExecute(60).Triggered.connect(this,MakeConnection1);
    }     
}
function MakeConnection1()
{
    if (counter > 0) 
    {
        console.ExecuteCommand("Disconnect");
        client.Login("localhost",2345,"Guest","","tcp");
        frame.DelayedExecute(300).Triggered.connect(this,MakeConnection2);
        counter--;
    }
    else
    { 
        print("Counter: " + counter);
        console.ExecuteCommand("Exit");
    }
}
function MakeConnection2()
{
    if (counter > 0) 
    {
        console.ExecuteCommand("Disconnect");
        client.Login("localhost",2346,"Guest","","tcp");
        frame.DelayedExecute(300).Triggered.connect(this,MakeConnection1);
        counter--;
    }
    else
    {
        print("Counter: " + counter);
        console.ExecuteCommand("Exit");
    }
}
    

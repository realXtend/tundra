//FileType: RealXtend naali Valgrind test-script
//Function: Start naali and quit the program.
//Date: 10.08.2011
//Made by: Jukka Vatjus-Anttila / Center for Internet Excellence

var doFirst = true;
// Start the process when FrameAPI sends first frameUpdate signal
frame.Updated.connect(StartProcess);

function StartProcess()
{
    // This function starts the whole process. Only run it at 1st frame and then set doFirst to false.
    if (doFirst)
    {
        doFirst = false;          
        // Wait for valgrind startup.
        var isserver = server.IsRunning() || server.IsAboutToStart();
        if (!isserver)
            frame.DelayedExecute(200).Triggered.connect(this,Disconnect);
    }
}
function Disconnect()
{
    // Just exit the program.
    console.ExecuteCommand("Exit");
}  


/*  This script is a simple way to keep your servers scene persistent without manual work.
    This is quite helpful when you want to run a 'production' server without manual labour
    to keep the content when you have to restart it etc. Also to have some auto storage in case
    of server crashes.

    1. Stores the current scene as is when server shuts down cleanly. This cant be promised if
       server crashes unexpectedly. For 'minimizing damages' on crashes you can use the interval store.
    2. Option to enable auto store of the scene with time interval.

    Configuration:
    - Startup Store/Restore: Change the bool 'startupEnabled' to true.
    - Interval Store: Change 'intervalEnabled' to true. 
      You can also adjust minutes how ofter auto store is done with 'intervalMinutes' variable.
    - Store Filename: You can change the filename 'storeFilename' where the scene is stored.
    
    Note to developers: Be careful not to commit 'true' values if you are using this script in your git clone.
*/

function AutoServerStore()
{
    this.startupEnabled = false;
    this.intervalEnabled = false;
    this.intervalMinutes = 60;
    this.storeFilename = "scene.txml";
}

AutoServerStore.prototype.Start = function()
{
    if (!server.IsAboutToStart())
        return;
        
    if (this.startupEnabled)
    {
        server.ServerStarted.connect(instance.ServerStarted);
        application.ExitRequested.connect(instance.ServerAboutToStop);
    }
    
    if (this.intervalEnabled)
    {
        this.intervalNow = 0.0;
        this.intervalStore = this.intervalMinutes * 60.0;
        frame.Updated.connect(instance.Update);
    }
}

AutoServerStore.prototype.Update = function(frametime)
{       
    instance.intervalNow += frametime;
    if (instance.intervalNow >= instance.intervalStore)
    {
        instance.intervalNow = 0.0;
        var successful = instance.SaveScene();
        if (successful)
            print("[AutoServerStore] Automatic store interval of", instance.intervalMinutes, "minutes passed. Storing scene to", instance.storeFilename);
    }
}

AutoServerStore.prototype.ServerStarted = function()
{
    var entCount = instance.LoadScene();
    if (entCount > 0)
        print("[AutoServerStore] Loaded", entCount, "entities from", instance.storeFilename);
}

AutoServerStore.prototype.ServerAboutToStop = function()
{
    var successful = instance.SaveScene();
    if (successful)
        print("[AutoServerStore] Storing scene before server shutdown to" + instance.storeFilename);
}

AutoServerStore.prototype.LoadScene = function()
{
    var entList = framework.DefaultScene().LoadSceneXML(this.storeFilename, true, false, 0);
    return entList.length;
}

AutoServerStore.prototype.SaveScene = function()
{
    var successful = framework.DefaultScene().SaveSceneXML(this.storeFilename);
    return successful;
}

var instance = new AutoServerStore();
instance.Start();

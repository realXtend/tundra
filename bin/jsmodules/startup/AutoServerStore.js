engine.ImportExtension("qt.core");
engine.ImportExtension("qt.gui");

engine.IncludeFile("jsmodules/lib/class.js");

/*  This script is a simple way to keep your servers scene persistent without manual work.
    This is quite helpful when you want to run a 'production' server without manual labour
    to keep the content when you have to restart it etc. Also to have some auto storage in case
    of server crashes.

    1. Can load the last stored scene on startup
    2. Can stores the current scene as is when server shuts down cleanly. This cant be promised if
       server crashes unexpectedly. For 'minimizing damages' on crashes you can use the interval store.
    3. Can enable auto store of the scene with time interval.

    Configuration:
    - This script will pop up configure dialogs if the values are not found from config, 
      so essentially on the irst run of servers on this machine. This will happen even on headless mode,
      see the todo about porting to console input instead of ui.
    - Do not modify this script file manually on your server. Instead modify the config file after its created
      if you want to change behaviour. You can find it in <user_appdata>/realXtend/configuration/tundra.ini in [server] section.
      
    \todo Port this to do input without UI, but use console for asking questions when tundra supports this kind of thing.
    \todo QInputDialog for getting integers from user is broken in js, see comments in the relecant section of the code. 
          intervalSaveMinutes can only be modified from the config once its written to appdata.
*/

var configFile = "tundra";
var configSection = "server";
        
var AutoServerStore = Class.extend
({
    init: function()
    {
        this.startupLoadEnabled = false;
        this.shutdownSaveEnabled = false;
        this.intervalSaveEnabled = false;
        this.intervalSaveMinutes = 60;
        this.sceneFilename = "scene.txml";
        
        this.intervalNow = 0.0;
        this.intervalStore = 0.0;
    },
    
    readConfig: function()
    {       
        // Watch out for the js boolean QVariant bug! Expect strings if bool.
        
        if (!config.HasValue(configFile, configSection, "startup_load_enabled"))
            config.Set(configFile, configSection, "startup_load_enabled", this.startupLoadEnabled);
        this.startupLoadEnabled = config.Get(configFile, configSection, "startup_load_enabled");
        if (this.startupLoadEnabled == "false")
            this.startupLoadEnabled = false;
        else
            this.startupLoadEnabled = true
        
        if (!config.HasValue(configFile, configSection, "shutdown_save_enabled"))
            config.Set(configFile, configSection, "shutdown_save_enabled", this.shutdownSaveEnabled);
        this.shutdownSaveEnabled = config.Get(configFile, configSection, "shutdown_save_enabled");
        if (this.shutdownSaveEnabled == "false")
            this.shutdownSaveEnabled = false;
        else
            this.shutdownSaveEnabled = true
            
        if (!config.HasValue(configFile, configSection, "interval_save_enabled"))
            config.Set(configFile, configSection, "interval_save_enabled", this.intervalSaveEnabled);
        this.intervalSaveEnabled = config.Get(configFile, configSection, "interval_save_enabled", this.intervalSaveEnabled);
        if (this.intervalSaveEnabled == "false")
            this.intervalSaveEnabled = false;
        else
            this.intervalSaveEnabled = true
            
        if (!config.HasValue(configFile, configSection, "interval_save_minutes"))
            config.Set(configFile, configSection, "interval_save_minutes", this.intervalSaveMinutes);
        this.intervalSaveMinutes = config.Get(configFile, configSection, "interval_save_minutes");
    },
    
    writeDefaultConfig: function()
    {
        config.Set(configFile, configSection, "startup_load_enabled", this.startupLoadEnabled);
        config.Set(configFile, configSection, "shutdown_save_enabled", this.shutdownSaveEnabled);
        config.Set(configFile, configSection, "interval_save_enabled", this.intervalSaveEnabled);
        config.Set(configFile, configSection, "interval_save_minutes", this.intervalSaveMinutes);
    },
    
    runConfigure: function()
    {
        // No ui for headless evev if this funtion will
        // never be invoked in headless mode, double check here.
        if (framework.IsHeadless())
            return;
        
        var dialogTitle = "AutoServerStore Configure Wizard";
        var messageBox = new QMessageBox(QMessageBox.Question, dialogTitle, "", QMessageBox.NoButton, 0, Qt.Tool);
        messageBox.addButton("Yes", QMessageBox.YesRole);
        messageBox.addButton("No", QMessageBox.NoRole);
        
        // Yes it's confusing...
        var yesAnswer = false;
        var noAnswer = true;
        var result = null;
        
        messageBox.text = "Load last stored scene on startup?";
        result = messageBox.exec();
        config.Set(configFile, configSection, "startup_load_enabled", (result == yesAnswer));
        
        messageBox.text = "Save scene before shutdown?";
        result = messageBox.exec();
        config.Set(configFile, configSection, "shutdown_save_enabled", (result == yesAnswer));
        
        messageBox.text = "Save scene every " + p_.intervalSaveMinutes + " minutes?";
        result = messageBox.exec();
        config.Set(configFile, configSection, "interval_save_enabled", (result == yesAnswer));
        config.Set(configFile, configSection, "interval_save_minutes", p_.intervalSaveMinutes);
        
        var dialogTitle = "AutoServerStore Configure Wizard";
        var messageBox = new QMessageBox(QMessageBox.Question, dialogTitle, "You will have to restart the\nserver for the changes to apply.", QMessageBox.NoButton, 0, Qt.Tool);
        messageBox.addButton("OK", QMessageBox.YesRole);
        messageBox.exec();
    },
    
    start: function()
    {
        this.readConfig();
        
        if (p_.startupLoadEnabled)
            server.ServerStarted.connect(this.serverStarted);
        
        if (p_.shutdownSaveEnabled)
            application.ExitRequested.connect(this.serverAboutToStop);
        
        if (p_.intervalSaveEnabled)
        {
            p_.intervalNow = 0.0;
            p_.intervalStore = p_.intervalSaveMinutes * 60.0;
            frame.Updated.connect(this.update);
        }
        
        var timer = new QTimer();
        timer.singleShot = true;
        timer.timeout.connect(this.addMenu);
        timer.timeout.connect(this.printReport);
        timer.start(500);
    },
    
    printReport: function()
    {
        var usingConfigFile = config.GetConfigFolder() + configFile + ".ini";
        // \todo Move this server info print somewhere to core.
        debug.Log("");
        debug.Log("Server information:");
        debug.Log("-- Port     : " + server.GetPort());
        debug.Log("-- Protocol : " + server.GetProtocol());
        debug.Log("");
        debug.Log("Auto Server Store configuration:");
        debug.Log("-- Config file : " + usingConfigFile);
        debug.Log("-- Scene file  : " + QDir.currentPath() + "/" + p_.sceneFilename);
        debug.Log("-- Load last scene on startup   : " + p_.startupLoadEnabled);
        debug.Log("-- Save scene state on shutdown : " + p_.shutdownSaveEnabled);
        debug.Log("-- Store scene every " + p_.intervalSaveMinutes + " min     : " + p_.intervalSaveEnabled);
    },
    
    addMenu: function()
    {
        if (!framework.IsHeadless())
        {
            var menuAction = ui.MainWindow().AddMenuAction("Server", "Configure Auto Store");
            menuAction.triggered.connect(p_.runConfigure);
        }
    },
    
    update: function(frametime)
    {
        p_.intervalNow += frametime;
        if (p_.intervalNow >= p_.intervalStore)
        {
            p_.intervalNow = 0.0;
            var successful = p_.saveScene();
            if (successful)
                debug.Log("Automatic store interval of " + p_.intervalSaveMinutes + " minutes passed. Storing scene to " + p_.sceneFilename);
        }
    },
    
    serverStarted: function()
    {
        var entCount = p_.loadScene();
        if (entCount > 0)
            debug.Log("Loaded " + entCount.toString() + " entities from " + p_.sceneFilename);
    },
    
    serverAboutToStop: function()
    {
        var successful = p_.saveScene();
        if (successful)
            debug.Log("Storing scene before server shutdown to " + p_.sceneFilename);
    },
    
    loadScene: function()
    {
        var entList = framework.Scene().GetDefaultSceneRaw().LoadSceneXML(p_.sceneFilename, true, false, 0);
        return entList.length;
    },
    
    saveScene: function()
    {
        var successful = framework.Scene().GetDefaultSceneRaw().SaveSceneXML(p_.sceneFilename);
        return successful;
    },
});

if (server.IsAboutToStart())
{
    var p_ = new AutoServerStore();
    p_.start();
}

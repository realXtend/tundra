
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
        var configureDone = config.Get(configFile, configSection, "auto_server_store_configured", false);
        if (!configureDone)
            this.runConfigure();
        
        // No read with the default values provided to be sure
        this.startupLoadEnabled = config.Get(configFile, configSection, "startup_load_enabled", this.startupLoadEnabled);
        this.shutdownSaveEnabled = config.Get(configFile, configSection, "shutdown_save_enabled", this.shutdownSaveEnabled);
        this.intervalSaveEnabled = config.Get(configFile, configSection, "interval_save_enabled", this.intervalSaveEnabled);
        this.intervalSaveMinutes = config.Get(configFile, configSection, "interval_save_minutes", this.intervalSaveMinutes);
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
        var dialogTitle = "AutoServerStore Configure Wizard";
        var messageBox = new QMessageBox(QMessageBox.Question, dialogTitle, "", QMessageBox.NoButton, 0, Qt.Tool);
        messageBox.addButton("Yes", QMessageBox.YesRole);
        messageBox.addButton("No", QMessageBox.NoRole);
        
        // Yes it's confusing...
        var yesAnswer = false;
        var noAnswer = true;
        
        messageBox.text = "Seems that this is the first time you are running the server,\nwould you like to run the AutoServerStore configure wizard?";
        var result = messageBox.exec();
        if (result == noAnswer)
        {
            messageBox.text = "Would you like me to remind you on next startup?\nAnswering 'No' will never show you dialogs again on this machine.";
            result = messageBox.exec();
            if (result == noAnswer)
                config.Set(configFile, configSection, "auto_server_store_configured", true);            
            this.writeDefaultConfig();
            return;
        }
        
        messageBox.text = "Load last stored scene on startup?";
        result = messageBox.exec();
        config.Set(configFile, configSection, "startup_load_enabled", (result == yesAnswer));
        
        messageBox.text = "Save scene before shutdown?";
        result = messageBox.exec();
        config.Set(configFile, configSection, "shutdown_save_enabled", (result == yesAnswer));
        
        messageBox.text = "Save scene every " + this.intervalSaveMinutes + " minutes?";
        result = messageBox.exec();
        config.Set(configFile, configSection, "interval_save_enabled", (result == yesAnswer));
        config.Set(configFile, configSection, "interval_save_minutes", this.intervalSaveMinutes);
        
        /*
        \todo Do something about this?
        
        QInputDialog.getInt() says its not a function and making it with new QInputDialog()
        is also buggy. You cant set the default value or the text that shows in the dialog on .exec()
        
        var inputIntervalMinutes = this.intervalSaveMinutes;
        if (result == yesAnswer)
        {
            var intInputDialog = new QInputDialog();
            intInputDialog.inputMode = QInputDialog.IntInput;
            intInputDialog.labelText = "How ofter to store scene during run? Insert interval in minutes:";
            intInputDialog.intValue = 60;
            
            intInputDialog.exec();
            inputIntervalMinutes = intInputDialog.intValue;
            print(inputIntervalMinutes);
            if (inputIntervalMinutes == null || inputIntervalMinutes <= 0)
                inputIntervalMinutes = this.intervalSaveMinutes;
            config.Set(configFile, configSection, "interval_save_minutes", inputIntervalMinutes);
        }
        */
        
        config.Set(configFile, configSection, "auto_server_store_configured", true);
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
        var entList = framework.Scene().GetDefaultSceneRaw().LoadSceneXML(this.sceneFilename, true, false, 0);
        return entList.length;
    },
    
    saveScene: function()
    {
        var successful = framework.Scene().GetDefaultSceneRaw().SaveSceneXML(this.sceneFilename);
        return successful;
    },
});

if (server.IsAboutToStart())
{
    var p_ = new AutoServerStore();
    p_.start();
}

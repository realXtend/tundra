// !ref: local://firstpersonmouseicon.png 

if (!server.IsRunning() && !framework.IsHeadless())
{
    function Crosshair()
    {
        engine.ImportExtension("qt.core");
        engine.ImportExtension("qt.gui");
        
        // Init
        this.is_active = false;
        this.is_ready = false;
        
        var iconAsset = asset.GetAsset("local://firstpersonmouseicon.png").get();
        if (iconAsset == null)
        {
            print("Could not find icon asset, cannot use first person mouse icon!");
            return;
        }
        
        var iconDiskSource = iconAsset.DiskSource();
        if (iconDiskSource == null || iconDiskSource == "")
        {
            print("Could not find icon asset disk source, cannot use first person mouse icon!");
            return;
        }
        
        var image = new QPixmap(8,8);
        image.load(iconDiskSource);
        this.cursor = new QCursor(image);
        
        // Mark as ready
        this.is_ready = true;
    }
    
    Crosshair.prototype.isActive = function()
    {
        return this.is_active;
    }

    Crosshair.prototype.show = function()
    {
        // Never put more than one of our icon to the stack
        if (this.is_active)
            return;

        if (this.is_ready)
        {
            QApplication.setOverrideCursor(this.cursor);
            this.is_active = true;
        }
    }
    
    Crosshair.prototype.hide = function()
    {
        if (!this.is_active)
            return;
            
        if (this.is_ready)
        {
            QApplication.restoreOverrideCursor();
            this.is_active = false;
        }
    }
}
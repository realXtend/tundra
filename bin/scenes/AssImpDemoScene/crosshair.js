// !ref: firstpersonmouseicon.png

if (!server.IsRunning() && !framework.IsHeadless())
{
    function Crosshair(useLabel)
    {
        // Init
        this.is_active = false;
        this.is_ready = false;
        this.isUsingLabel = useLabel;
        this.sideLength = 8;

        var iconAsset = asset.GetAsset("local://firstpersonmouseicon.png");
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

        var image = new QPixmap(this.sideLength, this.sideLength);
        image.load(iconDiskSource);

        if (!this.isUsingLabel)
        {
            this.cursor = new QCursor(image);
        }
        else
        {
            var label = new QLabel();
            label.resize(this.sideLength, this.sideLength);
            label.setPixmap(image);
            label.setStyleSheet("QLabel { background-color: transparent; }");
            this.proxy = new UiProxyWidget(label);
            this.proxy.focus = false;
            this.proxy.focusPolicy = Qt.NoFocus;
            //\todo: It would be nice to find a way in js to ignore mouse clicks on crosshair label.
            ui.AddProxyWidgetToScene(this.proxy);
            this.proxy.x = ui.GraphicsView().scene().width()/2 - this.sideLength/2;
            this.proxy.y = ui.GraphicsView().scene().height()/2 - this.sideLength/2;
            this.proxy.windowFlags = 0;
            this.proxy.visible = false;
            this.cursor = new QCursor(Qt.BlankCursor);
            ui.GraphicsView().scene().sceneRectChanged.connect(this, this.rectChanged);
        }
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
            if (this.isUsingLabel)
            {
                this.proxy.visible = true;
                input.SetMouseCursorVisible(false);
            }
            else
            {
                //\ note: SetMouseCursorVisible makes the nessesary mouse centering without producing double move events.
                //\       However, we need to show the cursor only, so additional checks are added in simpleavatar.js as well.
                input.SetMouseCursorVisible(false);
                QApplication.setOverrideCursor(this.cursor);
            }
            this.is_active = true;
        }
    }
    
    Crosshair.prototype.hide = function()
    {
        if (!this.is_active)
            return;

        if (this.is_ready)
        {
            if (this.isUsingLabel)
            {
                this.proxy.visible = false;
                input.SetMouseCursorVisible(true);
            }
            else
            {
                QApplication.restoreOverrideCursor();
                input.SetMouseCursorVisible(true);
            }
            this.is_active = false;
        }
    }

    Crosshair.prototype.rectChanged = function(rect)
    {
        this.proxy.pos = new QPointF(rect.width()/2 - this.sideLength/2, rect.height()/2 - this.sideLength/2);
    }
}

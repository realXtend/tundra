if (!framework.IsHeadless())
{
    engine.ImportExtension("qt.core");
    engine.ImportExtension("qt.gui");

    // Note: this will also attempt to load .txml files, which is not nice
    assetModule.LoadAllLocalAssetsWithSuffix("xml", "GenericAvatarXml");
    // The assets will be loaded during next frame, so delay the execution of populating the UI items.
    frame.DelayedExecute(0.5).Triggered.connect(PopulateAvatarUiMenu);
}

function MenuActionHandler(assetRef, index)
{
    this.assetName = assetRef;
} 
MenuActionHandler.prototype.triggered = function()
{
    var avatarEntity = scene.GetEntity("Avatar" + client.GetConnectionID());
    if (avatarEntity == null)
        return;
    var r = avatarEntity.avatar.appearanceRef;
    r.ref = this.assetName;
    avatarEntity.avatar.appearanceRef = r;
}

function PopulateAvatarUiMenu()
{
    var menu = ui.MainWindow().menuBar();
    var avatarMenu = menu.addMenu("&Avatar");
    
    var assets = asset.GetAllAssets();
    for (i in assets)         
    {
        if (assets[i].Type() == "GenericAvatarXml")
        {
            var assetName = assets[i].Name();
            // Check some hardcoded exceptions and do not show them
            if (assetName.indexOf(".txml") != -1)
                continue;
            if (assetName.indexOf("UserIdentities.xml") != -1)
                continue;
            var menuName = assetName.replace("local://", "").replace(".xml", "");
            var handler = new MenuActionHandler(assets[i].Name());
            avatarMenu.addAction(menuName).triggered.connect(handler, handler.triggered);
        }
    }
}

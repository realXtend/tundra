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

if (!framework.IsHeadless())
{
    engine.ImportExtension("qt.core");
    engine.ImportExtension("qt.gui");

    // Todo: for now scans the default storage only
    var assetList = asset.GetDefaultAssetStorage().GetAllAssetRefs();
    PopulateAvatarUiMenu(assetList);
}

function PopulateAvatarUiMenu(assetList)
{
    var menu = ui.MainWindow().menuBar();
    var avatarMenu = menu.addMenu("&Avatar");

    for (var i = 0; i < assetList.length; ++i)
    {
        var assetNameLower = assetList[i].toLowerCase();
        // Can not check the actual asset type from the ref only. But for now assume xml = avatar xml
        if (assetNameLower.indexOf(".xml") != -1)
        {
            var assetName = assetList[i];
            // Tidy up the displayed name a bit
            var menuName = assetName.replace("local://", "").replace(".xml", "");
            var handler = new MenuActionHandler(assetName);
            avatarMenu.addAction(menuName).triggered.connect(handler, handler.triggered);
        }
    }
}

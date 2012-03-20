var avatarMenu = null;

function MenuActionHandler(assetRef, index)
{
    this.assetName = assetRef;
}

MenuActionHandler.prototype.triggered = function()
{
    var avatarEntity = scene.GetEntityByName("Avatar" + client.connectionId);
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

    // Initial population
    PopulateAvatarUiMenu();

    // Connect to asset changes
    asset.AssetCreated.connect(OnAssetCreated);
}

function OnAssetCreated(asset)
{
    if (asset.Type() == "Avatar")
        PopulateAvatarUiMenu();
}

function PopulateAvatarUiMenu()
{
    var menu = ui.MainWindow().menuBar();
    if (avatarMenu == null)
        avatarMenu = menu.addMenu("&Avatar");

    avatarMenu.clear();

    var assetList = asset.GetAllAssetsOfType("Avatar");
    for (var i = 0; i < assetList.length; ++i)
    {
        var assetName = assetList[i].Name();
        var handler = new MenuActionHandler(assetName);
        avatarMenu.addAction(assetName).triggered.connect(handler, handler.triggered);
    }
}

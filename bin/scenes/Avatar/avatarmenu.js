var avatarMenu = null;

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

    // Connect to asset refs ready signal of existing storages (+ refresh them now)
    var assetStorages = asset.GetAssetStorages();
    for (var i = 0; i < assetStorages.length; ++i)
    {
        assetStorages[i].AssetRefsChanged.connect(PopulateAvatarUiMenu);
        assetStorages[i].RefreshAssetRefs();
    }
    // Connect to adding new storages
    asset.AssetStorageAdded.connect(OnAssetStorageAdded);
}

function OnAssetStorageAdded(storage)
{
    storage.AssetRefsChanged.connect(PopulateAvatarUiMenu);
    storage.RefreshAssetRefs();
}

function EndsWith(str, suffix)
{
    return str.indexOf(suffix) == (str.length - suffix.length);
}

function PopulateAvatarUiMenu()
{
    var menu = ui.MainWindow().menuBar();
    if (avatarMenu == null)
        avatarMenu = menu.addMenu("&Avatar");

    avatarMenu.clear();

    var assetStorages = asset.GetAssetStorages();
    for (var i = 0; i < assetStorages.length; ++i)
    {
        var assetList = assetStorages[i].GetAllAssetRefs();
        for (var j = 0; j < assetList.length; ++j)
        {
            var assetNameLower = assetList[j].toLowerCase();
            // Can not check the actual asset type from the ref only. But for now assume xml = avatar xml
            if ((EndsWith(assetNameLower, ".xml")) || (EndsWith(assetNameLower, ".avatar")))
            {
                var assetName = assetList[j];
                var handler = new MenuActionHandler(assetName);
                avatarMenu.addAction(assetName).triggered.connect(handler, handler.triggered);
            }
        }
    }
}

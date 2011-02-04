// Test script for the Asset API
// Run test with: viewer --run ./jsmodules/apitest/assets.js

var run = true;
frame.Updated.connect(Update);

function Update(frametime)
{
    if (run)
    {
        run = !run;
        RequestAsset("http://www.google.com/index.html", "Binary");
        RequestAsset("http://dl.dropbox.com/u/3589544/building/meshes/cube.mesh");
        RequestAsset("http://dl.dropbox.com/u/3589544/building/scenes/tundra/test1.txml");
        print("");
    }
}

function RequestAsset(ref, type)
{
    print("Requesting:", ref);
    transferptr = asset.RequestAsset(ref, type);
    transfer = transferptr.get();
    transfer.Downloaded.connect(DownloadReady);
    transfer.Loaded.connect(AssetReady);
}

function DownloadReady(/* IAssetTransfer* */ transfer) 
{
    data = transfer.GetRawData();
    print("Download ready");
    print("  >> Data len :", data.size(), "\n");
}

function AssetReady(/* IAssetPtr* */ assetptr)
{
    asset = assetptr.get();
    data = asset.GetRawData();
    print("Asset ready");
    print("  >> Class    :", asset);
    print("  >> Type     :", asset.Type());
    print("  >> Name     :", asset.Name());
    print("  >> Data len :", data.size(), "\n");
}



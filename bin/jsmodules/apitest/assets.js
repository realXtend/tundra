// Test script for the Asset API
// Run test with: viewer --run ./jsmodules/apitest/assets.js

var run = true;
frame.Updated.connect(Update);

// Define your 'uploadStorageUrl' here for the upload test to run.
// For example http://myserver.com/assets. The server needs to accept GET and POST requests.
var uploadStorageUrl = ""; 
var uploadStorageName = "Asset Test Storage";
var uploadTestFile = "./resources.cfg";

function Update(frametime)
{
    if (run)
    {
        run = !run;
        
        // Download tests
        // Please do add more download requests to the test if you want.
        RequestAsset("http://www.google.com/index.html", "Binary");
        RequestAsset("http://dl.dropbox.com/u/3589544/building/meshes/cube.mesh");
        RequestAsset("http://dl.dropbox.com/u/3589544/building/scenes/tundra/test1.txml");
        print("");
        
        // Upload tests
        if (uploadStorageUrl != "")
        {
            if (AddAssetStorage(uploadStorageUrl, uploadStorageName))
            {
                // You are free to add more upload requests to the test if you want.
                // You can also upload from memory directly with:
                // asset.UploadAssetFromFileInMemory(QByteArray data, QString storageName, QString assetName)
                UploadAsset(uploadTestFile, uploadStorageName, "MyNewResourceFile.cfg");
                print("");
            }
        }
        else
        {
            print("*** Cannot run upload tests!");
            print("*** You have to define 'uploadStorageUrl' to the script in /jsmodules/apitest/assets.js\n");
        }
    }
}

// Utility funcs

function AddAssetStorage(url, name)
{
    assetstorageptr = asset.AddAssetStorage(url, name);
    assetstorage = assetstorageptr.get();
    if (assetstorage != null)
    {
        print("Added asset storage");
        print(">>  Name :", assetstorage.Name());
        print(">>  URL  :", assetstorage.BaseURL(), "\n");
        return true;
    }
    else
    {
        print("Failed to add asset storage:", name, "-", url, "\n");
        return false;
    }
}

function ForgetAsset(assetRef)
{
    // Make AssetAPI forget this asset if already loaded in 
    // to the system and remove the disk cache entry.
    asset.ForgetAsset(assetRef, true);
}

// Downloading assets

function RequestAsset(ref, type)
{
    ForgetAsset(ref);
    
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
    print("  >> Source    :", transfer.GetSourceUrl());
    print("  >> Type      :", transfer.GetAssetType());
    print("  >> Data len  :", data.size(), "\n");
}

function AssetReady(/* IAssetPtr* */ assetptr)
{
    asset = assetptr.get();
    data = asset.GetRawData();
    print("Asset ready");
    print("  >> Class ptr :", asset);
    print("  >> Type      :", asset.Type());
    print("  >> Name      :", asset.Name());
    print("  >> Data len  :", data.size(), "\n");
}

// Uploading Assets

function UploadAsset(fileName, storageName, uploadName)
{
    print("Uploading:", fileName, "with destination name", uploadName);
    uploadtransferptr = asset.UploadAssetFromFile(fileName, storageName, uploadName);
    uploadtransfer = uploadtransferptr.get();
    if (uploadtransfer != null)
    {
        uploadtransfer.Completed.connect(UploadCompleted);
        uploadtransfer.Failed.connect(UploadFailed);
    }
    else
        print(" >> Failed to upload, AssetAPI returned a null AssetUploadTransferPtr");
}

function UploadCompleted(/* IAssetUploadTransfer* */ transfer)
{
    print("Upload completed");
    print("  >> New asset ref    :", transfer.AssetRef());
    print("  >> Destination name :", transfer.GetDesticationName(), "\n");
}

function UploadFailed(/* IAssetUploadTransfer* */ transfer)
{
    print("Upload failed");
    print("  >> File name   :", transfer.GetSourceFilename());
    print("  >> Destination :", transfer.GetDesticationName(), "\n");
}
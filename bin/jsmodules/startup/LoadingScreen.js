if (!framework.IsHeadless())
{
    print("aefawef");
    engine.ImportExtension("qt.core");
    engine.ImportExtension("qt.gui");

    var label = new QLabel();
    label.objectName = "InfoLabel";
    label.setStyleSheet("QLabel#InfoLabel { padding: 10px; background-color: rgba(230,230,230,100); border: 1px solid black; font-size: 16px; }");
    label.text = "Loading...";

    var proxy = ui.AddWidgetToScene(label);

    proxy.windowFlags = 0;
    proxy.visible = false;
    proxy.focusPolicy = Qt.NoFocus;
    proxy.x = ui.GraphicsView().width/2 - 250;
    proxy.y = ui.GraphicsView().height/2 - 40;
    label.resize(500, 200);

    var prevPendingTransfers = [];

    var numAssetsLoaded = 0;

    //var prevAssetsLeft = -1;
    //var maxAssetsLeft = asset.NumCurrentTransfers();

    var mostRecentCompletedTransfers = [];

    frame.Updated.connect(OnFrameUpdate);
}

ArrayDiff = function(a, b) {
    return a.filter(function(i) {return !(b.indexOf(i) > -1);});
};

function OnFrameUpdate()
{
    profiler.BeginBlock("LoadingScreen");
    var pendingAssetTransfers = asset.GetCurrentTransfers();
    var pendingTransfers = new Array();
    for(i in pendingAssetTransfers)
    {
        if (i != null && pendingAssetTransfers[i] != null)
        {
            var url = pendingAssetTransfers[i].SourceUrl();
            pendingTransfers.push(url);
        }
    }
    
    var newPendingTransfers = ArrayDiff(pendingTransfers, prevPendingTransfers);
    var completedTransfers = ArrayDiff(prevPendingTransfers, pendingTransfers);        
    prevPendingTransfers = pendingTransfers;
    if (newPendingTransfers.length > 0 || completedTransfers.length > 0)
    {
        numAssetsLoaded += completedTransfers.length;
        
        mostRecentCompletedTransfers = mostRecentCompletedTransfers.concat(completedTransfers);
        var numTransfersToShow = 3;
        if (mostRecentCompletedTransfers.length > numTransfersToShow)
            mostRecentCompletedTransfers.splice(0, mostRecentCompletedTransfers.length - numTransfersToShow);
           
        proxy.x = ui.GraphicsView().width/2 - 250;
        proxy.y = ui.GraphicsView().height/2 - 40;        
        if (pendingTransfers.length == 0)
            label.text = "Done.";
        else
            label.text = "Loading " + (numAssetsLoaded) + "/" + (numAssetsLoaded + pendingTransfers.length) + "...";

        if (pendingTransfers.length > 0)
            for(i = 1; i <= numTransfersToShow && mostRecentCompletedTransfers.length-i >= 0; i++)
                label.text = label.text + "\n" + mostRecentCompletedTransfers[mostRecentCompletedTransfers.length-i];

        if (pendingTransfers.length >= 5)
            proxy.visible = true;
        else if (pendingTransfers.length == 0)
            frame.DelayedExecute(1.5).Triggered.connect(HideLoadingScreen);
    }
    profiler.EndBlock();
}

function HideLoadingScreen()
{
    var numAssetsLeft = asset.NumCurrentTransfers();
    if (numAssetsLeft == 0)
        proxy.visible = false;    
}

function OnScriptDestroyed()
{
    if (!framework.IsHeadless())
        label.deleteLater();
}

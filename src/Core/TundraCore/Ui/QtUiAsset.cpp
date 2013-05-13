// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "QtUiAsset.h"
#include "UiProxyWidget.h"
#include "Framework.h"
#include "AssetAPI.h"
#include "UiAPI.h"
#include "LoggingFunctions.h"

#include <QUiLoader>
#include <QByteArrayMatcher>
#include <QUrl>
#include <QFile>

#include "MemoryLeakCheck.h"

QtUiAsset::QtUiAsset(AssetAPI *owner, const QString &type_, const QString &name_) :
    IAsset(owner, type_, name_)
{
}

QtUiAsset::~QtUiAsset()
{
}

bool QtUiAsset::DeserializeFromData(const u8 *data, size_t numBytes, bool /*allowAsynchronous*/)
{
    refs.clear();
    originalData.clear();

    if (!data || numBytes == 0)
    {
        LogError("QtUiAsset::DeserializeFromData: null data.");
        return false;
    }

    originalData.insert(originalData.end(), data, data + numBytes);

    // If we are in headless mode, do not mark refs as dependency as it will
    // just spawn unneeded resources to the asset api without the actual need for them.
    // Note that this means we assume you will not show any ui on a headless run, while you
    // still could without the main window.
    if (assetAPI->IsHeadless())
    {
        assetAPI->AssetLoadCompleted(Name());
        return true;
    }

    // Now, go through the contents of the .ui asset file and look for references to other assets (ui images),
    // and mark those down into an auxiliary structure. Later, before letting Qt load up the .ui file, 
    // we replace these network url refs with the actual disk source of each asset.
    std::string uiAssetFile(data, data + numBytes);

    bool found = true;
    std::string::const_iterator start = uiAssetFile.begin();
    const std::string::const_iterator end = uiAssetFile.end();
    while(found)
    {
        smatch r;
        // Try to find lines of form 'url(http://myserver.com/asset.png)' or 'url(someotherkindofassetref)'
        // There may be double-quotes in the form of '"' or '&quot;', which the regex needs to take into account.
        // The regex ignores all redundant whitespace, i.e. 'url(   "   local://myasset.png &quot;  )' is also matched.
        regex e("url\\(\\s*((&quot;)|\\\")?\\s*(.*?)\\s*((&quot;)|\\\")?\\s*\\)");
        found = regex_search(start, end, r, e);
        if (found)
        {
            AssetRef ref;
            ref.index = r[3].first - uiAssetFile.begin();
            ref.length = r[3].second - r[3].first;
            ref.parsedRef = std::string(r[3].first, r[3].second).c_str();
            ref.parsedRef.replace(QRegExp("[\'\"\\t\\n\\r\\v\\f\\a]"), "");
            ref.parsedRef = assetAPI->ResolveAssetRef(Name(), ref.parsedRef);
            ref.encloseInQuotes = true;
            ref.type = assetAPI->GetResourceTypeFromAssetRef(ref.parsedRef);

            // Check if AssetAPI has previous knowledge of this ref (eg via storage auto discovery),
            // if not we enforce "Texture" type to "Binary" so its not unnecessarily loaded to Ogre
            if (ref.type == "Texture" && !assetAPI->GetAsset(ref.parsedRef).get())
                ref.type = "Binary";

            refs.push_back(ref);
            start = r[0].second;
        }
    }

    found = true;
    start = uiAssetFile.begin();
    while(found)
    {
        smatch r;
        // Try to find lines of form '<pixmap>http://myserver.com/asset.png</pixmap>' or '<pixmap>someotherkindofassetref</pixmap>'
        // There may be double-quotes in the form of '"' or '&quot;', which the regex needs to take into account.
        // The regex ignores all redundant whitespace, i.e. '< pixmap > local://myasset.png </ pixmap >' is also matched.
//        regex e("\\<\\s*(pixmap|normaloff|normalon|disabledoff|disabledon|activeoff|activeon|selectedoff|selectedon)\\s*\\>\\s*((&quot;)|\")?\\s*(.*?)\\s*((&quot;)|\")?\\s*\\<\\s*/(pixmap|normaloff|normalon|disabledoff|disabledon|activeoff|activeon|selectedoff|selectedon)\\s*\\>");

        // Parse all <pixmap>imagefile.png</pixmap>, or <normalon>imagefile.png</normalon>, and other tags (pixmap, normalon, normaloff, disabledon, disabledoff, activeoff, activeon, selectedon, selectedoff).
        // Note that this mechanism has been detected to not support alpha for the images, so better to use CSS styling for them instead (i.e. QPushButton:hover and QPushButton:pressed).
        regex e(">([^<>]+?\\.png)<");
        found = regex_search(start, end, r, e);
        if (found)
        {
            AssetRef ref;
            //const int cg = 3; // The capture group of the URL in the above commented out regex.
            const int cg = 1;
            ref.index = r[cg].first - uiAssetFile.begin();
            ref.length = r[cg].second - r[cg].first;
            ref.parsedRef = std::string(r[cg].first, r[cg].second).c_str();
            ref.parsedRef.replace(QRegExp("[\'\"\\t\\n\\r\\v\\f\\a]"), "");
            ref.parsedRef = assetAPI->ResolveAssetRef(Name(), ref.parsedRef);
            ref.encloseInQuotes = false;
            ref.type = assetAPI->GetResourceTypeFromAssetRef(ref.parsedRef);

            // Check if AssetAPI has previous knowledge of this ref (eg via storage auto discovery),
            // if not we enforce "Texture" type to "Binary" so its not unnecessarily loaded to Ogre
            if (ref.type == "Texture" && !assetAPI->GetAsset(ref.parsedRef).get())
                ref.type = "Binary";

            refs.push_back(ref);
            start = r[0].second;
        }
    }

    // Resolve the refs now
    for (unsigned i = 0; i < refs.size(); ++i)
        refs[i].parsedRef = assetAPI->ResolveAssetRef(Name(), refs[i].parsedRef);

    assetAPI->AssetLoadCompleted(Name());
    return true;
}

bool QtUiAsset::SerializeTo(std::vector<u8> &data, const QString &/*serializationParameters*/) const
{
    data = originalData;
    return data.size() > 0;
}

void QtUiAsset::DoUnload()
{
    // Unloading a ui asset does not have any meaning, as it's not a GPU resource and it doesn't have any kind of decompress/unpack step.
    // Implementation for this asset is a no-op.
}

std::vector<AssetReference> QtUiAsset::FindReferences() const
{
    std::vector<AssetReference> assetRefs;
    for(size_t i = 0; i < refs.size(); ++i)
        assetRefs.push_back(AssetReference(refs[i].parsedRef, refs[i].type));
    return assetRefs;
}

QByteArray QtUiAsset::GetRefReplacedAssetData() const
{
    if (originalData.size() == 0)
        return QByteArray();
    QByteArray refRewrittenData((const char *)&originalData[0], (int)originalData.size());

    // The AssetRef indices need to be adjusted with an offset after rewriting each ref, since the lengths of the refs change in the file.
    // This variable tracks the accumulated byte offset that takes this into account.
    int indexAdjustment = 0; 

    for(size_t i = 0; i < refs.size(); ++i)
    {
        QString assetDiskSource = "";
        AssetPtr asset = assetAPI->GetAsset(refs[i].parsedRef);
        if (!asset.get())
        {
            LogError("QtUiAsset::GetRefReplacedAssetData: Asset not found from asset system even when it was marked as a dependency earlier, skipping: " + refs[i].parsedRef);
        }
        else
        {
            assetDiskSource = asset->DiskSource();
            if (assetDiskSource.isEmpty())
                LogWarning("QtUiAsset::GetRefReplacedAssetData: Asset \"" + refs[i].parsedRef + "\" does not have a disk source. The UI asset \"" + Name() + "\" will not be able to use it.");
        }
        assetDiskSource = assetDiskSource.trimmed();
        if (!assetDiskSource.isEmpty() && QFile::exists(assetDiskSource))
        {
            QByteArray refAsByteArray;
            if (refs[i].encloseInQuotes)
                refAsByteArray = (QString("\"") + assetDiskSource + QString("\"")).toUtf8();
            else
                refAsByteArray = (assetDiskSource).toUtf8();
            refRewrittenData.replace(refs[i].index + indexAdjustment, refs[i].length, refAsByteArray);
            indexAdjustment += refAsByteArray.length() - refs[i].length;
        }
        else
        {
            LogWarning("QtUiAsset::GetRefReplacedAssetData: Asset disk source does not exist, skipping: " + refs[i].parsedRef);
        }
    }
    return refRewrittenData;
}

bool QtUiAsset::IsLoaded() const
{
    return originalData.size() > 0;
}

QWidget *QtUiAsset::Instantiate(bool addToScene, QWidget *parent)
{
    if (!IsLoaded())
    {
        LogError("QtUiAsset::Instantiate: Cannot instantiate an unloaded UI Asset \"" + Name() + "\"!");
        return 0;
    }

    // Get the asset data with the assetrefs replaced to point to the disk sources on the current local system.
    QByteArray data = GetRefReplacedAssetData();

    QUiLoader loader;
    QDataStream dataStream(&data, QIODevice::ReadOnly);
    QWidget *widget = loader.load(dataStream.device(), parent);

    if (!widget)
    {
        LogError("QtUiAsset::Instantiate: Failed to instantiate widget from UI asset \"" + Name() + "\"!");
        return 0;
    }

    if (addToScene)
    {
        UiProxyWidget *proxy = assetAPI->GetFramework()->Ui()->AddWidgetToScene(widget);
        if (!proxy)
            LogError("QtUiAsset::Instantiate: Failed to add widget to main QGraphicsScene in UI asset \"" + Name() + "\"!");
    }

    return widget;
}
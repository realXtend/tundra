// For conditions of distribution and use, see copyright notice in license.txt

#include "DebugOperatorNew.h"
#include "QtUiAsset.h"

#include "AssetAPI.h"

#include "LoggingFunctions.h"
DEFINE_POCO_LOGGING_FUNCTIONS("QtUiAsset")

#include <boost/regex.hpp>

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

bool QtUiAsset::DeserializeFromData(const u8 *data, size_t numBytes)
{
    refs.clear();
    originalData.clear();

    if (!data || numBytes == 0)
        return false;

    originalData.insert(originalData.end(), data, data + numBytes);

    // If we are in headless mode, do not mark refs as dependency as it will
    // just spawn unneeded resources to the asset api without the actual need for them.
    // Note that this means we assume you will not show any ui on a headless run, while you
    // still could without the main window.
    if (assetAPI->IsHeadless())
        return true;

    // Now, go through the contents of the .ui asset file and look for references to other assets (ui images),
    // and mark those down into an auxiliary structure. Later, before letting Qt load up the .ui file, 
    // we replace these network url refs with the actual disk source of each asset.
    std::string uiAssetFile(data, data + numBytes);

    bool found = true;
    std::string::const_iterator start = uiAssetFile.begin();
    const std::string::const_iterator end = uiAssetFile.end();
    while(found)
    {
        boost::smatch r;
        // Try to find lines of form 'url(http://myserver.com/asset.png)' or 'url(someotherkindofassetref)'
        // There may be double-quotes in the form of '"' or '&quot;', which the regex needs to take into account.
        // The regex ignores all redundant whitespace, i.e. 'url(   "   local://myasset.png &quot;  )' is also matched.
        boost::regex e("url\\(\\s*((&quot;)|\\\")?\\s*(.*?)\\s*((&quot;)|\\\")?\\s*\\)");
        found = boost::regex_search(start, end, r, e);
        if (found)
        {
            AssetRef ref;
            ref.index = r[3].first - uiAssetFile.begin();
            ref.length = r[3].second - r[3].first;
            ref.parsedRef = std::string(r[3].first, r[3].second).c_str();
            ref.parsedRef.replace(QRegExp("[\'\"\\t\\n\\r\\v\\f\\a]"), "");
            refs.push_back(ref);
            start = r[0].second;
        }
    }

    found = true;
    start = uiAssetFile.begin();
    while(found)
    {
        boost::smatch r;
        // Try to find lines of form '<pixmap>http://myserver.com/asset.png</pixmap>' or '<pixmap>someotherkindofassetref</pixmap>'
        // There may be double-quotes in the form of '"' or '&quot;', which the regex needs to take into account.
        // The regex ignores all redundant whitespace, i.e. '< pixmap > local://myasset.png </ pixmap >' is also matched.
        boost::regex e("\\<\\s*pixmap\\s*\\>\\s*((&quot;)|\")?\\s*(.*?)\\s*((&quot;)|\")?\\s*\\<\\s*/pixmap\\s*\\>");
        found = boost::regex_search(start, end, r, e);
        if (found)
        {
            AssetRef ref;
            ref.index = r[3].first - uiAssetFile.begin();
            ref.length = r[3].second - r[3].first;
            ref.parsedRef = std::string(r[3].first, r[3].second).c_str();
            ref.parsedRef.replace(QRegExp("[\'\"\\t\\n\\r\\v\\f\\a]"), "");
            refs.push_back(ref);
            start = r[0].second;
        }
    }

    return true;
}

bool QtUiAsset::SerializeTo(std::vector<u8> &data, const QString &serializationParameters )
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
    {
        assetRefs.push_back(AssetReference(refs[i].parsedRef));
    }
    return assetRefs;
}

QByteArray QtUiAsset::GetRefReplacedAssetData() const
{
    if (originalData.size() == 0)
        return QByteArray();
    QByteArray refRewrittenData((const char *)&originalData[0], originalData.size());

    // The AssetRef indices need to be adjusted with an offset after rewriting each ref, since the lengths of the refs change in the file.
    // This variable tracks the accumulated byte offset that takes this into account.
    int indexAdjustment = 0; 

    for(size_t i = 0; i < refs.size(); ++i)
    {
        QString assetDiskSource = "";
        AssetPtr asset = assetAPI->GetAsset(refs[i].parsedRef);
        if (!asset.get())
        {
            LogError("ReplaceAssetReferences: Asset not found from asset system even when it was marked as a dependency earlier, skipping: " + refs[i].parsedRef.toStdString());
        }
        else
        {
            assetDiskSource = asset->DiskSource();
            if (assetDiskSource.isEmpty())
                LogWarning("ReplaceAssetReferences: Asset disk source empty, skipping: " + refs[i].parsedRef.toStdString());
        }
        assetDiskSource = assetDiskSource.trimmed();
        if (!assetDiskSource.isEmpty() && QFile::exists(assetDiskSource))
        {
            QByteArray refAsByteArray = (QString("\"") + assetDiskSource + QString("\"")).toUtf8();
            refRewrittenData.replace(refs[i].index + indexAdjustment, refs[i].length, refAsByteArray);
            indexAdjustment += refAsByteArray.length() - refs[i].length;
        }
        else
        {
            LogWarning("ReplaceAssetReferences: Asset disk source does not exist, skipping: " + refs[i].parsedRef.toStdString());
        }
    }
    return refRewrittenData;
}

bool QtUiAsset::IsLoaded() const
{
    return originalData.size() > 0;
}

// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "QtUiAsset.h"
#include "AssetAPI.h"

#include <QByteArrayMatcher>
#include <QUrl>

#include "LoggingFunctions.h"
DEFINE_POCO_LOGGING_FUNCTIONS("QtUiAsset")

QtUiAsset::QtUiAsset(AssetAPI *owner, const QString &type_, const QString &name_) :
    IAsset(owner, type_, name_)
{
    patterns_ << "http://" << "https://" << "local://";
}

QtUiAsset::~QtUiAsset() 
{
}

bool QtUiAsset::DeserializeFromData(const u8 *data, size_t numBytes)
{
    refs_.clear();
    data_.clear();
    data_.insert(data_.begin(), data, data + numBytes);   

    // Find references
    QByteArray dataQt((const char*)&data_[0], data_.size());
    if (!dataQt.isNull() && !dataQt.isEmpty())
    {
        QByteArrayMatcher matcher;
        foreach(QByteArray pattern, patterns_)
        {
            int from = 0;
            int indexStart = 0;
            matcher.setPattern(pattern);
            while (indexStart != -1)
            {
                indexStart = matcher.indexIn(dataQt, from);
                if (indexStart == -1)
                    break;
                int indexStop = dataQt.indexOf(")", indexStart);
                QUrl ref(dataQt.mid(indexStart, indexStop - indexStart), QUrl::TolerantMode);
                if (ref.isValid())
                    refs_.push_back(AssetReference(ref.toString()));
                from = indexStop;
            }
        }
    }
    else
        LogError("FindReferences: Could not process .ui file content, error: data was null.");

    return true;
}

bool QtUiAsset::SerializeTo(std::vector<u8> &data, const QString &serializationParameters )
{
    if (data_.empty())
        return false;
    data.clear();
    data = data_;
    return true;
}

void QtUiAsset::DoUnload()
{
}

std::vector<AssetReference> QtUiAsset::FindReferences() const
{   
    return refs_;
}

int QtUiAsset::ReplaceAssetReferences(QByteArray &uiData)
{
    int replacedRefs = 0;
    for(uint i=0; i<refs_.size(); ++i)
    {
        QString assetRef = refs_[i].ref;
        AssetPtr asset = assetAPI->GetAsset(assetRef);
        if (!asset.get())
        {
            LogError("ReplaceAssetReferences: Asset not found from asset system even when it was marked as a dependency earlier, skipping: " + assetRef.toStdString());
            continue;
        }
        QString refDiskSource = asset->DiskSource();
        if (refDiskSource.isEmpty())
        {
            LogWarning("ReplaceAssetReferences: Asset disk source empty, skipping: " + assetRef.toStdString());
            continue;
        }
        if (!QFile::exists(refDiskSource))
        {
            LogWarning("ReplaceAssetReferences: Asset disk source does not exist, skipping: " + assetRef.toStdString());
            continue;
        }
        uiData.replace(assetRef, refDiskSource.toStdString().c_str());
        replacedRefs++;
    }
    return replacedRefs;
}

bool QtUiAsset::IsDataValid() const
{
    return !data_.empty();
}

QByteArray QtUiAsset::GetRawData() const
{
    QByteArray returnData;
    if (!IsDataValid())
        return returnData;
    returnData = QByteArray((const char*)&data_[0], data_.size());
    return returnData;
}

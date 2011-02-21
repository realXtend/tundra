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
    patterns_ << "http://" << "https://" << "local://" << "file://";
    invalid_ref_chars_ << " " << "\n" << "\r" << "\t" << "\v" << "\f" << "\a";
}

QtUiAsset::~QtUiAsset() 
{
}

bool QtUiAsset::DeserializeFromData(const u8 *data, size_t numBytes)
{
    refs_.clear();
    data_.clear();
    data_.insert(data_.begin(), data, data + numBytes);   

    // If we are in headless mode, do not mark refs as dependency as it will
    // just spawn unneeded resources to the asset api without the actual need for them.
    // Note that this means we assume you will not show any ui on a headless run, while you
    // still could without the main window.
    if (assetAPI->IsHeadless())
        return true;

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

                // ")" due to Qt style sheets have this syntax for example: 
                // some-image-property: url({"|'}pattern{"|'});
                int indexStop1 = dataQt.indexOf(")", indexStart);
                // "<" due to normal qt xml defining images, for example: 
                // <iconset><normaloff>pattern</normaloff>pattern</iconset>
                int indexStop2 = dataQt.indexOf("<", indexStart);
                if (indexStop1 == -1 && indexStop2 == -1)
                {
                    from = indexStart + 1;
                    continue;
                }
                int indexStop = indexStop1;
                if (indexStop2 < indexStop || indexStop == -1)
                    indexStop = indexStop2;
                from = indexStop;
                if (indexStop < indexStart)
                    continue;

                // Asset reference validation
                QString stringRef = dataQt.mid(indexStart, indexStop - indexStart).trimmed();
                
                // Ignore ui elements that have patterns as text, for example:
                // <widget><property name="text"><string>pattern</string></property></widget>
                if (indexStop == indexStop2)
                {
                    int indexStop3 = dataQt.indexOf(">", indexStart);
                    QString xmlEndTag = dataQt.mid(indexStop2, (indexStop3 - indexStop2) + 1);
                    if (xmlEndTag.toLower() == "</string>")
                        continue;
                }

                // Remove the last ' or " from url({"|'}pattern{"|'}) before the found )
                QChar lastChar = stringRef.at(stringRef.length()-1);
                if (lastChar == '\'' || lastChar == '\"')
                    stringRef.chop(1);
                if (stringRef.isEmpty() || stringRef.isNull())
                    continue;

                // Check for not wanted characters
                foreach(QString invalid, invalid_ref_chars_)
                    if (stringRef.contains(invalid))
                        continue;

                // QUrl validation, in tolerant mode
                QUrl ref(stringRef, QUrl::TolerantMode);
                if (!ref.isValid())
                    continue;

                // Seems legit, add it
                refs_.push_back(AssetReference(ref.toString()));
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

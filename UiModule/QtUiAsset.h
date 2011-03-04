// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_QtUiAsset_h
#define incl_UiModule_QtUiAsset_h

#include "AssetFwd.h"
#include "IAsset.h"
#include "AssetReference.h"

#include <QList>
#include <QByteArray>
#include <QStringList>

class QtUiAsset : public IAsset
{
    Q_OBJECT

public:
    QtUiAsset(AssetAPI *owner, const QString &type_, const QString &name_);

    virtual ~QtUiAsset();

    /// Loads this asset by deserializing it from the given data. The data pointer that is passed in is never null, and numBytes is always greater than zero.
    virtual bool DeserializeFromData(const u8 *data, size_t numBytes);

    /// Saves this asset to the given data buffer. Returns true on success. If this asset is unloaded, will return false.
    /// @param serializationParameters Optional parameters for the actual asset type serializer that specifies custom options on how to perform the serialization.
    virtual bool SerializeTo(std::vector<u8> &data, const QString &serializationParameters = "");

    /// Private-implementation of the unloading of an asset.
    virtual void DoUnload();

    /// Find dependencies from the asset
    virtual std::vector<AssetReference> FindReferences() const;

public slots:
    /// Call this to replace asset references of the original ui data with our absolute cache file paths.
    /// Called from UiService::LoadFromFile(). This function will modify the input data and return the number of replaced references.
    /// @param QByteArray Reference to the original ui file data. This function will modify its contents
    /// @return int Number of replaced references
    int ReplaceAssetReferences(QByteArray &uiData);

    /// Returns if this assets data is valid.
    bool IsDataValid() const;

    /// Return the raw data of this asset.
    QByteArray GetRawData() const;

private:
    QList<QByteArray> patterns_;
    QStringList invalid_ref_chars_;

    std::vector<u8> data_;
    std::vector<AssetReference> refs_;
};

#endif

// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "TundraCoreApi.h"
#include "AssetFwd.h"
#include "IAsset.h"
#include "AssetReference.h"

#include <QList>
#include <QByteArray>
#include <QStringList>

/// Contains the data for a Qt .ui file.
class TUNDRACORE_API QtUiAsset : public IAsset
{
    Q_OBJECT

public:
    QtUiAsset(AssetAPI *owner, const QString &type_, const QString &name_);

    virtual ~QtUiAsset();

    /// Loads this asset by deserializing it from the given data. The data pointer that is passed in is never null, and numBytes is always greater than zero.
    virtual bool DeserializeFromData(const u8 *data, size_t numBytes, bool allowAsynchronous);

    /// Saves this asset to the given data buffer. Returns true on success. If this asset is unloaded, will return false.
    /// @param serializationParameters Optional parameters for the actual asset type serializer that specifies custom options on how to perform the serialization.
    virtual bool SerializeTo(std::vector<u8> &data, const QString &serializationParameters = "") const;

    /// Find dependencies from the asset
    virtual std::vector<AssetReference> FindReferences() const;

public slots:
    /// Call this to get a copy of the .ui asset file contents, that has each asset reference replaced with the actual disk source of each asset in the system.
    /// Called from UiAPI::LoadFromFile(). This function will return a modified copy of the original .ui file.
    QByteArray GetRefReplacedAssetData() const;

    /// Returns true if this UI asset is valid and loaded in memory.
    bool IsLoaded() const;

    /// Creates an instance of this .ui file.
    /// @param addToScene If true, this widget is added to the main UI. Otherwise the window is shown externally.
    QWidget *Instantiate(bool addToScene, QWidget *parent);

private:
    /// Unloading a ui asset does not have any meaning, as it's not a GPU resource and it doesn't have any kind of decompress/unpack step.
    /// Implementation for this asset is a no-op.
    virtual void DoUnload();

    struct AssetRef
    {
        /// The byte index where this assetref occurs in the .ui file.
        int index;
        /// The byte length of the ref string.
        int length;
        /// Stores a parsed/sanitized version of the assetRef, to be fed to the AssetAPI for retrieval.
        QString parsedRef;
        /// If true, this asset ref is present in a CSS style block in the .ui file. Otherwise, it is a value of an XML element. The former
        /// need to be enclosed in quotes, the latter can't, or otherwise Qt .ui image loading will fail.
        bool encloseInQuotes;

        /// Asset type
        QString type;
    };

    /// Stores the found asset refs in the .ui file.
    std::vector<AssetRef> refs;

    /// Stores the original asset data. This data doesn't have the asset refs rewritten.
    std::vector<u8> originalData;
};

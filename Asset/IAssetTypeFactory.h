// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_AssetFactory_h
#define incl_Asset_AssetFactory_h

#include <QObject>
#include <QHash>
#include <QString>
#include <QStringList>

#include "AssetFwd.h"

class IAssetTypeFactory : public QObject
{

Q_OBJECT

public:
    virtual ~IAssetTypeFactory() {}

public slots:
    /// Returns the type of assets this asset type factory can create. The assetType_ must be set by the implementer of the subclass.
    /// @return QString asset type for this factory.
    virtual QString GetType() const { return assetType_; } 

    /// Returns a the supported file extensions for this factory. The list must be filled by the implementer of the subclass.
    /// @return QStringList List of file extensions this factory supports.
    virtual QStringList GetSupportedFormats() const { return supportedFileFormats_; }

    /// Creates a new asset of the given type that is initialized to the "empty" asset of this type.
    /// @param name The name to give for this asset.
    virtual AssetPtr CreateEmptyAsset(AssetAPI *owner, const char *name) = 0;

protected:
    /// Asset type of this factory.
    QString assetType_;

    /// Supported file formats. Fill this with file extensions this factory supports, without the '.' in from, for example "png" or "wav".
    QStringList supportedFileFormats_;
};

#endif

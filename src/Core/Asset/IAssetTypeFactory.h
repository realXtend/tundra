// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include <QObject>
#include <QString>
#include "AssetFwd.h"

/// A common interface for factories which instantiate assets of different types.
class IAssetTypeFactory : public QObject
{
public:
    virtual ~IAssetTypeFactory() {}

    /// Returns the type of assets this asset type factory can create.
    virtual QString Type() const = 0;

    /// Creates a new asset of the given type that is initialized to the "empty" asset of this type.
    /// @param name The name to give for this asset.
    virtual AssetPtr CreateEmptyAsset(AssetAPI *owner, const QString &name) = 0;
};


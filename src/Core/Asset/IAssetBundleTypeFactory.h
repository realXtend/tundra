// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include <QObject>
#include <QStringList>
#include "AssetFwd.h"

/// A common interface for factories which instantiate asset bundles of different types.
class IAssetBundleTypeFactory : public QObject
{
public:
    virtual ~IAssetBundleTypeFactory() {}

    /// Returns the type of this the asset bundles this factory produces.
    virtual QString Type() const = 0;

    /// Returns the file extensions types of asset bundles this asset type factory can create.
    virtual QStringList TypeExtensions() const = 0;

    /// Creates a new asset bundle of the given type that is initialized to the "empty" asset bundle of this type.
    /** @param owner AssetAPI ptr.
        @param name The name to give for this asset bundle. */
    virtual AssetBundlePtr CreateEmptyAssetBundle(AssetAPI *owner, const QString &name) = 0;
};

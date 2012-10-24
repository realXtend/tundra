// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "IAssetBundleTypeFactory.h"

/// This class provides a factory for archived bundle assets like zip files.
class ArchiveBundleFactory : public IAssetBundleTypeFactory
{
Q_OBJECT
    
public:
    ArchiveBundleFactory();

    virtual QString Type() const;
    virtual QStringList TypeExtensions() const;
    virtual AssetBundlePtr CreateEmptyAssetBundle(AssetAPI *owner, const QString &name);

private:
    QStringList typesExtensions_;
};

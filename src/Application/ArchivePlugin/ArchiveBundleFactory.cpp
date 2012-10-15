// For conditions of distribution and use, see copyright notice in LICENSE

#include "ArchiveBundleFactory.h"
#include "ZipAssetBundle.h"

ArchiveBundleFactory::ArchiveBundleFactory()
{
    typesExtensions_ << ".zip";
}

QString ArchiveBundleFactory::Type() const
{
    return "Archive";
}

QStringList ArchiveBundleFactory::TypeExtensions() const
{
    return typesExtensions_;
}

AssetBundlePtr ArchiveBundleFactory::CreateEmptyAssetBundle(AssetAPI *owner, const QString &name)
{
    if (name.endsWith(".zip", Qt::CaseInsensitive))
        return AssetBundlePtr(new ZipAssetBundle(owner, Type(), name));
    return AssetBundlePtr();
}

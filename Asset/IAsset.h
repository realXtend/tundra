// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_IAsset_h
#define incl_Asset_IAsset_h

#include <QObject>
#include <vector>

#include "AssetFwd.h"

class IAsset : public QObject
{
    Q_OBJECT

public:
    virtual ~IAsset() {}

    /// Stores this asset to disk to the given file.
    void SaveToFile(QString filename);

    /// Returns all the assets this asset refers to (but not the references those assets refer to).
    std::vector<IAsset*> FindReferences() const;

    /// Returns all the assets this asset refers to, and the assets those assets refer to, and so on.
    std::vector<IAsset*> FindReferencesDeep() const;

    /// Stores the raw asset bytes for this asset.
    std::vector<char> rawAssetData;

private:
    /// Specifies the provider this asset was downloaded from.
    IAssetProvider *provider;
};

#endif

// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_AvatarModule_AvatarDescAsset_h
#define incl_AvatarModule_AvatarDescAsset_h

#include <QString>
#include <QObject>
#include <vector>
#include <boost/shared_ptr.hpp>
#include "IAsset.h"

class AvatarDescAsset : public IAsset
{
    Q_OBJECT;
public:
    AvatarDescAsset(const QString &type_, const QString &name_)
    :IAsset(type_, name_)
    {
    }

    virtual bool LoadFromFileInMemory(const u8 *data, size_t numBytes);
    virtual std::vector<AssetReference> FindReferences() const { return assetReferences; }

    /// Stores the downloaded avatar appearance XML file as raw .xml data.
    QString avatarAppearanceXML;

    /// Specifies all the parsed asset references in the XML file.
    std::vector<AssetReference> assetReferences;

    void ParseReferences();
};

typedef boost::shared_ptr<AvatarDescAsset> AvatarDescAssetPtr;

#endif

// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_BinaryAsset_h
#define incl_Asset_BinaryAsset_h

#include <boost/shared_ptr.hpp>
#include "IAsset.h"

class BinaryAsset : public IAsset
{
    Q_OBJECT;
public:
    BinaryAsset(const QString &type_, const QString &name_)//, const QString &ref_)
    :IAsset(type_, name_)//, ref_)
    {
    }

    virtual bool LoadFromFileInMemory(const u8 *data_, size_t numBytes)
    {
        data.clear();
        data.insert(data.end(), data_, data_ + numBytes);

        return true;
    }

    virtual std::vector<AssetReference> FindReferences() const
    {
        return std::vector<AssetReference>();
    }

    std::vector<u8> data;
};

typedef boost::shared_ptr<BinaryAsset> BinaryAssetPtr;

#endif

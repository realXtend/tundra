// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_Script_ScriptAsset_h
#define incl_EC_Script_ScriptAsset_h

#include <boost/shared_ptr.hpp>
#include "IAsset.h"

class ScriptAsset : public IAsset
{
    Q_OBJECT;
public:
    ScriptAsset(const QString &type_, const QString &name_)
    :IAsset(type_, name_)
    {
    }

    virtual bool LoadFromFileInMemory(const u8 *data, size_t numBytes)
    {
        QByteArray arr((const char *)data, numBytes);
        arr.append('\0');
        scriptContent = arr;

        return true;
    }

    virtual std::vector<AssetReference> FindReferences() const
    {
        return std::vector<AssetReference>();
    }

    QString scriptContent;
};

typedef boost::shared_ptr<ScriptAsset> ScriptAssetPtr;

#endif

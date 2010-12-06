// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_Script_ScriptAsset_h
#define incl_EC_Script_ScriptAsset_h

#include <boost/shared_ptr.hpp>
#include "IAsset.h"

class ScriptAsset : public IAsset
{
    Q_OBJECT;
public:
    ScriptAsset(const QString &type_, const QString &name_)//, const QString &ref_)
    :IAsset(type_, name_)//, ref_)
    {
    }

    virtual bool LoadFromFileInMemory(const u8 *data, size_t numBytes)
    {
        QByteArray arr((const char *)data, numBytes);
        arr.append('\0');
        scriptContent = arr;

        ParseReferences();

        return true;
    }

    virtual std::vector<AssetReference> FindReferences() const { return references; }

    QString scriptContent;

    /// The asset references specified by this asset are specified in the above scriptContent data,
    /// but we cache them here on loading to quicken the access if they're needed several times.
    /// This also performs validation-on-load.
    std::vector<AssetReference> references;

    void ParseReferences();
};

typedef boost::shared_ptr<ScriptAsset> ScriptAssetPtr;

#endif

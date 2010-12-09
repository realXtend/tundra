// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_Script_ScriptAsset_h
#define incl_EC_Script_ScriptAsset_h

#include <boost/shared_ptr.hpp>
#include "IAsset.h"

class ScriptAsset : public IAsset
{
    Q_OBJECT;
public:
    ScriptAsset(AssetAPI *owner, const QString &type_, const QString &name_)
    :IAsset(owner, type_, name_)
    {
    }

    ~ScriptAsset();

    virtual void Unload();

    virtual bool DeserializeFromData(const u8 *data, size_t numBytes);

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

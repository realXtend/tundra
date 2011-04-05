// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_Script_ScriptAsset_h
#define incl_EC_Script_ScriptAsset_h

#include <boost/shared_ptr.hpp>
#include "IAsset.h"

class ScriptAsset : public IAsset
{
    Q_OBJECT;
public:
    ScriptAsset(AssetAPI *owner, const QString &type_, const QString &name_) :
        IAsset(owner, type_, name_)
    {
    }

    ~ScriptAsset();
    
    /// Load script asset from memory
    virtual bool DeserializeFromData(const u8 *data, size_t numBytes);

    /// Load script asset into memory
    virtual bool SerializeTo(std::vector<u8> &dst, const QString &serializationParameters);

    /// Unload script asset
    virtual void DoUnload();

    /// Return found asset references inside the script
    virtual std::vector<AssetReference> FindReferences() const { return references; }

    /// The asset references specified by this asset are specified in the above scriptContent data,
    /// but we cache them here on loading to quicken the access if they're needed several times.
    /// This also performs validation-on-load.
    std::vector<AssetReference> references;

    QString scriptContent;

    bool IsLoaded() const;

private slots:
    /// Parse internal references from script
    void ParseReferences();
};

typedef boost::shared_ptr<ScriptAsset> ScriptAssetPtr;

#endif

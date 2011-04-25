// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_Script_ScriptAssetFactory_h
#define incl_EC_Script_ScriptAssetFactory_h

#include "IAssetTypeFactory.h"
#include "ScriptAsset.h"

class ScriptAssetFactory : public IAssetTypeFactory
{

Q_OBJECT;

public:
    explicit ScriptAssetFactory()
    {
        assetType_ = "Script";
        supportedFileFormats_ << "js" << "py";
    }

    virtual AssetPtr CreateEmptyAsset(AssetAPI *owner, const char *name) { return AssetPtr(new ScriptAsset(owner, GetType(), name)); }
};

#endif

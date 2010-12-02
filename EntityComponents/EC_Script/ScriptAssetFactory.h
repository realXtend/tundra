// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_Script_ScriptAssetFactory_h
#define incl_EC_Script_ScriptAssetFactory_h

#include "IAssetTypeFactory.h"
#include "ScriptAsset.h"

class ScriptAssetFactory : public IAssetTypeFactory
{
    Q_OBJECT;
public:

    virtual QString Type() const { return "Script"; }

    virtual AssetPtr CreateEmptyAsset(const char *name) { return AssetPtr(new ScriptAsset(Type(), name)); }
};

#endif

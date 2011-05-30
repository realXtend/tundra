// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include "IAssetTypeFactory.h"
#include "ScriptAsset.h"

class ScriptAssetFactory : public IAssetTypeFactory
{
    Q_OBJECT;
public:

    virtual QString Type() const { return "Script"; }

    virtual AssetPtr CreateEmptyAsset(AssetAPI *owner, const char *name) { return AssetPtr(new ScriptAsset(owner, Type(), name)); }
};


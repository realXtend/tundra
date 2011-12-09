// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "IAssetTypeFactory.h"
#include "ScriptAsset.h"

/// Asset factory for script assets.
class ScriptAssetFactory : public IAssetTypeFactory
{
    Q_OBJECT

public:
    virtual QString Type() const { return "Script"; }

    virtual AssetPtr CreateEmptyAsset(AssetAPI *owner, const QString &name) { return AssetPtr(new ScriptAsset(owner, Type(), name)); }
};

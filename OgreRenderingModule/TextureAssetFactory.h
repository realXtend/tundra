// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderingModule_TextureAssetFactory_h
#define incl_OgreRenderingModule_TextureAssetFactory_h

#include "IAssetTypeFactory.h"
#include "TextureAsset.h"

class TextureAssetFactory : public IAssetTypeFactory
{
    Q_OBJECT;
public:

    virtual QString Type() const { return "Texture"; }

    virtual AssetPtr CreateEmptyAsset(const char *name) { return AssetPtr(new TextureAsset(Type(), name)); }
};

#endif

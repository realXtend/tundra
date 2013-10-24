// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

template<typename T>
shared_ptr<T> AssetAPI::FindAsset(QString assetRef) const
{
    return dynamic_pointer_cast<T>(FindAsset(assetRef));
}

template<typename T>
std::vector<shared_ptr<T> > AssetAPI::AssetsOfType() const
{
    std::vector<shared_ptr<T> > ret;
    for(AssetMap::const_iterator i = assets.begin(); i != assets.end(); ++i)
    {
        shared_ptr<T> asset = dynamic_pointer_cast<T>(i->second);
        if (asset)
            ret.push_back(asset);
    }
    return ret;
}

template<typename T>
shared_ptr<T> AssetAPI::AssetProvider() const
{
    const std::vector<AssetProviderPtr> providers = AssetProviders();
    for(size_t i = 0; i < providers.size(); ++i)
    {
        shared_ptr<T> provider = dynamic_pointer_cast<T>(providers[i]);
        if (provider)
            return provider;
    }
    return shared_ptr<T>();
}

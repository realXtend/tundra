// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

template<typename T>
boost::shared_ptr<T> AssetAPI::AssetProvider() const
{
    std::vector<AssetProviderPtr> providers = GetAssetProviders();
    for(size_t i = 0; i < providers.size(); ++i)
    {
        boost::shared_ptr<T> provider = boost::dynamic_pointer_cast<T>(providers[i]);
        if (provider.get())
            return provider;
    }
    return boost::shared_ptr<T>();
}

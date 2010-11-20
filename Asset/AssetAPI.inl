// For conditions of distribution and use, see copyright notice in license.txt

template<typename T>
boost::shared_ptr<T> AssetAPI::GetAssetProvider()
{
    std::vector<Foundation::AssetProviderPtr> providers = GetAssetProviders();
    for(size_t i = 0; i < providers.size(); ++i)
    {
        T *provider = dynamic_cast<T*>(providers[i].get());
        if (provider)
            return boost::shared_ptr<T>(provider);
    }
    return boost::shared_ptr<T>();
}


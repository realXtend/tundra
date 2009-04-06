// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_AssetInterface_h
#define incl_Interfaces_AssetInterface_h

namespace Foundation
{
    class AssetInterface;
    typedef boost::shared_ptr<AssetInterface> AssetPtr;
    
    class MODULE_API AssetInterface
    {
    public:
        AssetInterface() {}
        virtual ~AssetInterface() {}
        
        virtual const std::string GetId() = 0;
        virtual Core::asset_type_t GetType() = 0;
        virtual const std::string& GetTypeName() = 0;
        virtual Core::uint GetSize() = 0;
        virtual const Core::u8* GetData() = 0;
    };
}

#endif


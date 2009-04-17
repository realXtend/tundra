// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_AssetInterface_h
#define incl_Interfaces_AssetInterface_h

namespace Foundation
{
    class AssetInterface;
    typedef boost::shared_ptr<AssetInterface> AssetPtr;
    
    //! interface for assets
    class MODULE_API AssetInterface
    {
    public:
        //! default constructor
        AssetInterface() {}
        
        //! default destructor
        virtual ~AssetInterface() {}
        
        //! returns asset id. Can be for example UUID in textual form
        virtual const std::string GetId() = 0;
        
        //! returns asset type
        virtual Core::asset_type_t GetType() = 0;
        
        //! returns asset type in text form
        virtual const std::string& GetTypeName() = 0;
        
        //! returns asset data size
        virtual Core::uint GetSize() = 0;
        
        //! returns asset data
        virtual const Core::u8* GetData() = 0;
    };
}

#endif


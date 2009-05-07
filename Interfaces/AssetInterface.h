// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_AssetInterface_h
#define incl_Interfaces_AssetInterface_h

namespace Foundation
{
    class AssetInterface;
    typedef boost::shared_ptr<AssetInterface> AssetPtr;
    
    //! Interface for assets
    class MODULE_API AssetInterface
    {
    public:
        //! Default constructor
        AssetInterface() {}
        
        //! Default destructor
        virtual ~AssetInterface() {}
        
        //! Returns asset id. Can be for example UUID in text form
        virtual const std::string& GetId() = 0;
        
        //! Returns asset type in text form
        virtual const std::string& GetType() = 0;
        
        //! Returns asset data size
        virtual Core::uint GetSize() = 0;
        
        //! Returns asset data
        virtual const Core::u8* GetData() = 0;
    };
}

#endif


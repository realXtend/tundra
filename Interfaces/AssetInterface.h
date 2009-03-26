// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_AssetInterface_h
#define incl_Interfaces_AssetInterface_h

namespace Foundation
{
    class AssetInterface;
    typedef boost::shared_ptr<AssetInterface> AssetPtr;
    
    class AssetInterface
    {
    public:
        AssetInterface()  {}
        virtual ~AssetInterface() {}
    };
    
}

#endif


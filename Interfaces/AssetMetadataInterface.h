// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_AssetMetadataInterface_h
#define incl_Interfaces_AssetMetadataInterface_h

#include <map>
#include <string>
#include <ctime>

namespace Foundation
{
    //! Maps methods in form <method name> <method url>
    typedef std::map<std::string, std::string> MethodMap;

    //! Asset meta data like 
    //! For more info read http://opensimulator.org/wiki/AssetServerProposal/ClientDocs
    class AssetMetadataInterface
    {
    public:
        virtual ~AssetMetadataInterface() {};

        //! Returns asset id
        virtual const std::string& GetId() const = 0;

        //! Returns asset name
        virtual const std::string& GetName() const = 0;

        //! Returns description of asset
        virtual const std::string& GetDescription() const = 0;

        //! Returns creation date of asset
        //!   format: unix time stamp is now used
        virtual const time_t  GetCreationDate() const = 0;

        //! Returns creation date of asset as string
        virtual std::string GetCreationDateString() const = 0;

        //! Returns content type
        virtual const std::string& GetContentType() const = 0;

        //! Return SHA1 hash of asset data
        //! Format: Hexadecimal (40 char length string)
        virtual std::string GetHashSHA1() const = 0; 

        //! Return true id asset is temporary
        virtual bool IsTemporary() const = 0;

        //! Return methods as <method, url> map 
        virtual const MethodMap& GetMethods() const = 0;
    };
}

#endif

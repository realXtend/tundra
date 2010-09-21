// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLogic_AvatarExporter_h
#define incl_RexLogic_AvatarExporter_h

#include "ThreadTask.h"

namespace AvatarModule
{
    //! An asset to export.
    /*! Note: hash has to be calculated before submitting for export, and many assets
        with duplicate same hash should not exist!
     */
    class ExportAsset
    {
    public:
        //! Asset data
        std::vector<u8> data_;
        //! Base64-encoded SHA1 hash of data
        std::string hash_;
        
        void CalculateHash();
    };
    
    //! Asset map for export, keyed by filename
    typedef std::map<std::string, ExportAsset> ExportAssetMap;
    
    //! Request for avatar export
    class AvatarExporterRequest : public Foundation::ThreadTaskRequest
    {
    public:
        //! User account
        std::string account_;
        //! Authentication server to use (note: auth.server tells the storage address to use)
        std::string authserver_;
        //! Password
        std::string password_;
        //! Avatar description
        std::string avatar_xml_;
        //! Assets to be exported
        ExportAssetMap assets_;
    };

    //! Result for avatar export
    class AvatarExporterResult : public Foundation::ThreadTaskResult
    {
    public:
        //! Success flag
        bool success_;
        //! Message to be displayed (if any)
        std::string message_;
    };

    typedef boost::shared_ptr<AvatarExporterRequest> AvatarExporterRequestPtr;
    typedef boost::shared_ptr<AvatarExporterResult> AvatarExporterResultPtr;
    
    //! Threadtask that handles avatar export to legacy avatar storage.
    /*! Also handles periodically refreshing the authentication session hash, so that export stays legal.
     */
    class AvatarExporter : public Foundation::ThreadTask
    {
    public:
        AvatarExporter();
        
        virtual void Work();
        
    private:
        //! Perform the export
        void ProcessRequest(AvatarExporterRequestPtr request, AvatarExporterResultPtr result);
        
        //! Login to authentication & get new sessionhash
        bool LoginToAuthentication(const std::string& account, const std::string& authserver, const std::string& password, std::string& sessionhash, std::string& avatarurl, std::string& error);
    };
    
    typedef boost::shared_ptr<AvatarExporter> AvatarExporterPtr;
}

#endif

// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "AvatarExporter.h"
#include "RexLogicModule.h"
#include "LLSDUtilities.h"
#include "XMLRPCEPI.h"
#include "Poco/MD5Engine.h"

using namespace RexTypes;

namespace RexLogic
{
    AvatarExporter::AvatarExporter() : ThreadTask("AvatarExport")
    {
    }
    
    void AvatarExporter::Work()
    {
        AvatarExporterResultPtr result(new AvatarExporterResult());
        
        AvatarExporterRequestPtr request = GetNextRequest<AvatarExporterRequest>();
        if (request)
            ProcessRequest(request, result);
        else
        {
            result->message_ = "No valid export request";
            result->success_ = false;
        }
        
        SetResult<AvatarExporterResult>(result);
    }
    
    void AvatarExporter::ProcessRequest(AvatarExporterRequestPtr request, AvatarExporterResultPtr result)
    {
        if (request->avatar_xml_.empty())
        {
            result->message_ = "Empty avatar description";
            result->success_ = false;
            return;
        }
        
        // Replace < > so that xmlrpc call doesn't get confused
        ReplaceSubstring(request->avatar_xml_, "<", "&lt;");
        ReplaceSubstring(request->avatar_xml_, ">", "&gt;");

        std::string sessionhash;
        std::string avatarurl;
        std::string error;
        
        // Authenticate first, to get an uptodate sessionhash
        if (!LoginToAuthentication(request->account_, request->authserver_, request->password_, sessionhash, avatarurl, error))
        {
            result->message_ = "Failed authentication on avatar export: " + error;
            result->success_ = false;
            return;
        }
        RexLogicModule::LogInfo("Sessionhash: " + sessionhash);
        RexLogicModule::LogInfo("Avatar url: " + avatarurl);
        
        result->success_ = true;
    }
    
    bool AvatarExporter::LoginToAuthentication(const std::string& account, const std::string& authserver, const std::string& password, std::string& sessionhash, std::string& avatarurl, std::string& error)
    {
        sessionhash = "";
        avatarurl = "";
        error = "";
        
        std::size_t pos = authserver.rfind(":");
        std::string auth_port = "";
        std::string auth_address = "";

        Poco::MD5Engine md5_engine;

        md5_engine.update(password.c_str(), password.size());
        std::string password_hash = "$1$" + md5_engine.digestToHex(md5_engine.digest());

        if (pos != std::string::npos)
        {
            auth_port = authserver.substr(pos+1);
            auth_address = authserver.substr(0,pos);
        }
        else
        {
            auth_port = "10001";
            auth_address = authserver;
        }
        
        XmlRpcEpi call;
        try
        {
            call.Connect(auth_address, auth_port);
            call.CreateCall("ClientAuthentication");
            call.AddMember("account", account);
            call.AddMember("passwd", password_hash);
            call.Send();
            sessionhash = call.GetReply<std::string>("sessionHash");
            avatarurl = call.GetReply<std::string>("avatarStorageUrl");
            if (sessionhash.empty())
            {
                error = call.GetReply<std::string>("message");
                return false;
            }
            return true;
        }
        catch(XmlRpcException& ex)
        {
            error = ex.what();
            return false;
        }
        
        return true;
    }
}
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
        std::string avatar_url;
        std::string error;
        
        std::size_t pos = request->authserver_.rfind(":");
        if (pos != std::string::npos)
            request->authserver_ = request->authserver_.substr(0,pos) + ":" + request->authserver_.substr(pos+1);
        else
            request->authserver_ = request->authserver_ + ":10001";

        // Authenticate first, to get an uptodate sessionhash
        if (!LoginToAuthentication(request->account_, request->authserver_, request->password_, sessionhash, avatar_url, error))
        {
            result->message_ = "Failed authentication on avatar export: " + error;
            result->success_ = false;
            return;
        }
        
        RexLogicModule::LogInfo("Authenticated for export");
        
        std::string export_url = avatar_url;
        ReplaceSubstring(export_url, "/avatar/", "/xmlrpc/");
        
        XmlRpcEpi call;
        
        try
        {
            call.Connect(export_url);
            call.CreateCall("StoreAvatarHash");
            call.AddMember("account", request->account_);
            call.AddMember("UserServer", request->authserver_);
            call.AddMember("sessionhash", sessionhash);
            call.AddMember("generic xml", request->avatar_xml_);
            call.Send();
            
            //! \todo handle avatar assets & missing items
            
            if (call.HasReply("StoreAvatarHash"))
            {
                RexLogicModule::LogInfo("Export: StoreAvatarHash = " + call.GetReply<std::string>("StoreAvatarHash"));
            }
            if (call.HasReply("Error"))
            {
                RexLogicModule::LogInfo("Export: Error = " + call.GetReply<std::string>("Error"));
            }
            if (call.HasReply("MissingItems"))
            {
                RexLogicModule::LogInfo("Export: Missing items");
            }
        }
        catch(XmlRpcException& ex)
        {
            result->message_ = ex.what();
            result->success_ = false;
            return;
        }
        
        result->success_ = true;
    }
    
    bool AvatarExporter::LoginToAuthentication(const std::string& account, const std::string& authserver, const std::string& password, std::string& sessionhash, std::string& avatar_url, std::string& error)
    {
        sessionhash = "";
        avatar_url = "";
        error = "";
        
        Poco::MD5Engine md5_engine;

        md5_engine.update(password.c_str(), password.size());
        std::string password_hash = "$1$" + md5_engine.digestToHex(md5_engine.digest());
        
        XmlRpcEpi call;
        try
        {
            call.Connect(authserver);
            call.CreateCall("ClientAuthentication");
            call.AddMember("account", account);
            call.AddMember("passwd", password_hash);
            call.Send();
            if (call.HasReply("sessionHash"))
            {
                sessionhash = call.GetReply<std::string>("sessionHash");
                avatar_url = call.GetReply<std::string>("avatarStorageUrl");
                return true;
            }
            else
            {
                if (call.HasReply("message"))
                    error = call.GetReply<std::string>("message");
                else
                    error = "Login failed";
                return false;
            }
        }
        catch(XmlRpcException& ex)
        {
            error = ex.what();
            return false;
        }
        
        return true;
    }
}
// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "AvatarExporter.h"
#include "RexLogicModule.h"
#include "LLSDUtilities.h"
#include "XMLRPCEPI.h"
#include "Poco/MD5Engine.h"
#include "Poco/SHA1Engine.h"
#include "Poco/Base64Encoder.h"

using namespace RexTypes;

// Time in which to reauthenticate if export has lasted long, in seconds
static const Core::Real REAUTHENTICATION_TIME = 60.0f;

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
        
        // Calculate hashes from assets
        ExportHashMap hashes = CalculateAssetHashes(request->assets_);
        
        // Replace < > so that xmlrpc call doesn't get confused
        ReplaceSubstring(request->avatar_xml_, "<", "&lt;");
        ReplaceSubstring(request->avatar_xml_, ">", "&gt;");
        
        std::size_t pos = request->authserver_.rfind(":");
        if (pos != std::string::npos)
            request->authserver_ = request->authserver_.substr(0,pos) + ":" + request->authserver_.substr(pos+1);
        else
            request->authserver_ = request->authserver_ + ":10001";

        // Authenticate first, to get an uptodate sessionhash
        boost::timer export_timer; // For checking if we should refresh the hash
        std::string sessionhash;
        std::string avatar_url;
        std::string error;
        if (!LoginToAuthentication(request->account_, request->authserver_, request->password_, sessionhash, avatar_url, error))
        {
            result->message_ = "Failed authentication on avatar export: " + error;
            result->success_ = false;
            return;
        }
        
        RexLogicModule::LogInfo("Authenticated for export");
        
        std::string export_url = avatar_url;
        ReplaceSubstring(export_url, "/avatar/", "/xmlrpc/");
        
        try
        {
            // Export avatar
            RexLogicModule::LogInfo("Exporting avatar xml");
            
            XmlRpcEpi call;
            call.Connect(export_url);
            call.CreateCall("StoreAvatarHash");
            call.AddMember("account", request->account_);
            call.AddMember("UserServer", request->authserver_);
            call.AddMember("sessionhash", sessionhash);
            call.AddMember("generic xml", request->avatar_xml_);
            
            ExportHashMap::const_iterator i = hashes.begin();
            while (i != hashes.end())
            {
                //! Here, the hash is the key and asset name is the value
                call.AddMember(i->second.c_str(), i->first);
                ++i;
            }
            
            call.Send();
            
            // Export assets
            bool reexport = false;
            if (call.HasReply("MissingItems"))
            {
                reexport = true;
                RexLogicModule::LogInfo("Exporting assets");
                std::vector<std::string> items = call.GetVectorReply<std::string>("MissingItems");
                for (Core::uint i = 0; i < items.size(); ++i)
                {
                    // Re-login to get a fresh hash if export has lasted long
                    if (export_timer.elapsed() > REAUTHENTICATION_TIME)
                    {
                        export_timer.restart();
                        std::string temp;
                        if (!LoginToAuthentication(request->account_, request->authserver_, request->password_, sessionhash, temp, error))
                        {
                            result->message_ = "Failed re-authentication on avatar export: " + error;
                            result->success_ = false;
                            return;
                        }
                    }
                
                    std::string name = items[i];
                    ExportAssetMap::const_iterator asset = request->assets_.find(name);
                    ExportHashMap::const_iterator hash = hashes.find(name);
                    if ((asset != request->assets_.end()) && (hash != hashes.end()))
                    {
                        XmlRpcEpi asset_call;
                        asset_call.Connect(export_url);
                        asset_call.CreateCall("StoreItem");
                        asset_call.AddMember("account", request->account_);
                        asset_call.AddMember("UserServer", request->authserver_);
                        asset_call.AddMember("sessionhash", sessionhash);
                        asset_call.AddMember("itemname", name);
                        asset_call.AddMember("hashcode", hash->second);
                        
                        const std::vector<Core::u8>& data = asset->second;
                        XMLRPC_VectorAppendBase64(asset_call.GetXMLRPCCall()->GetParamList(), "binaries", (const char*)&data[0], data.size());
                        
                        asset_call.Send();
                        
                        std::string status = asset_call.GetReply<std::string>("StoreItem");
                        if (status != "succeeded")
                        {
                            result->message_ = "Failed to store asset " + name;
                            result->success_ = false;
                            return;
                        }
                    }
                    else
                    {
                        RexLogicModule::LogInfo("Storage requested unknown missing asset");
                    }
                }
            }
            else
            {
                if (call.HasReply("Error"))
                {
                    result->message_ = call.GetReply<std::string>("Error");
                    result->success_ = false;
                    return;
                }
                if (call.HasReply("StoreAvatarHash"))
                {
                    result->message_ = call.GetReply<std::string>("StoreAvatarHash");
                    if (result->message_ != "failed")
                        result->success_ = true;
                    else
                        result->success_ = false;
                    return;
                }
            }
            
            if (reexport)
            {
                RexLogicModule::LogInfo("Re-exporting avatar xml");
                
                // Re-login to get a fresh hash if export has lasted long
                if (export_timer.elapsed() > REAUTHENTICATION_TIME)
                {
                    export_timer.restart();
                    std::string temp;
                    if (!LoginToAuthentication(request->account_, request->authserver_, request->password_, sessionhash, temp, error))
                    {
                        result->message_ = "Failed re-authentication on avatar export: " + error;
                        result->success_ = false;
                        return;
                    }
                }
                
                // Re-export avatar
                XmlRpcEpi call;
                call.Connect(export_url);
                call.CreateCall("StoreAvatarHash");
                call.AddMember("account", request->account_);
                call.AddMember("UserServer", request->authserver_);
                call.AddMember("sessionhash", sessionhash);
                call.AddMember("generic xml", request->avatar_xml_);
                
                ExportHashMap::const_iterator i = hashes.begin();
                while (i != hashes.end())
                {
                    call.AddMember(i->second.c_str(), i->first);
                    ++i;
                }
                
                call.Send();
                
                if (call.HasReply("Error"))
                {
                    result->message_ = call.GetReply<std::string>("Error");
                    result->success_ = false;
                    return;
                }
                if (call.HasReply("StoreAvatarHash"))
                {
                    result->message_ = call.GetReply<std::string>("StoreAvatarHash");
                    if (result->message_ != "failed")
                        result->success_ = true;
                    else
                        result->success_ = false;
                    return;
                }
            }
        }
        catch(XmlRpcException& ex)
        {
            result->message_ = ex.what();
            result->success_ = false;
            return;
        }
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
    
    ExportHashMap AvatarExporter::CalculateAssetHashes(const ExportAssetMap& assets)
    {
        ExportHashMap hashes;
        
        ExportAssetMap::const_iterator i = assets.begin();

        while (i != assets.end())
        {
            Poco::SHA1Engine sha1_engine;
            sha1_engine.update(&i->second[0], i->second.size());
            const Poco::DigestEngine::Digest& digest = sha1_engine.digest();
            
            std::stringstream sstream;
            Poco::Base64Encoder encoder(sstream);
            for (Core::uint j = 0; j < digest.size(); ++j)
                encoder << digest[j];
            
            hashes[i->first] = sstream.str();
            ++i;
        }

        return hashes;
    }
    
}
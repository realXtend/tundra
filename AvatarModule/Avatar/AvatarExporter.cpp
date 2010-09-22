// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "AvatarModule.h"
#include "Avatar/AvatarExporter.h"

#include "LLSDUtilities.h"
#include "XmlRpcEpi.h"

#include "Poco/MD5Engine.h"
#include "Poco/SHA1Engine.h"
#include "Poco/Base64Encoder.h"

#include <QByteArray>
#include <QUrl>

using namespace RexTypes;

// Time in which to reauthenticate if export has lasted long, in seconds
static const float REAUTHENTICATION_TIME = 60.0f;

namespace Avatar
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
        ReplaceSubstringInplace(request->avatar_xml_, "<", "&lt;");
        ReplaceSubstringInplace(request->avatar_xml_, ">", "&gt;");
        
        QUrl authserver_url(QString::fromStdString(request->authserver_));
        if (authserver_url.port() == -1)
        {
            authserver_url.setPort(10001);
            request->authserver_ = authserver_url.toString().toStdString();
        }
        
        // Authenticate first to get an uptodate sessionhash
        boost::timer export_timer; // For checking if we should refresh the hash further on
        std::string sessionhash;
        std::string avatar_url;
        std::string error;
        if (!LoginToAuthentication(request->account_, request->authserver_, request->password_, sessionhash, avatar_url, error))
        {
            result->message_ = "Failed authentication on avatar export: " + error;
            result->success_ = false;
            return;
        }
        
        AvatarModule::LogInfo("Authenticated for export");
        
        std::string export_url = ReplaceSubstring(avatar_url, "/avatar/", "/xmlrpc/");
        
        try
        {
            // Export avatar
            AvatarModule::LogInfo("Exporting avatar xml");
            
            XmlRpcEpi call;
            call.Connect(export_url);
            call.CreateCall("StoreAvatarHash");
            call.AddMember("account", request->account_);
            call.AddMember("UserServer", request->authserver_);
            call.AddMember("sessionhash", sessionhash);
            call.AddMember("generic xml", request->avatar_xml_);
            
            ExportAssetMap::const_iterator i = request->assets_.begin();
            while (i != request->assets_.end())
            {
                //! Here, the hash is the key and asset name is the value
                call.AddMember(i->second.hash_.c_str(), i->first);
                ++i;
            }
            
            call.Send();
            
            // Export assets
            bool reexport = false;
            if (call.HasReply("MissingItems"))
            {
                reexport = true;
                AvatarModule::LogInfo("Exporting assets");
                std::vector<std::string> items = call.GetVectorReply<std::string>("MissingItems");
                for (uint i = 0; i < items.size(); ++i)
                {
                    std::string name = items[i];
                    ExportAssetMap::const_iterator asset = request->assets_.find(name);
                    if (asset != request->assets_.end())
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
                        
                        XmlRpcEpi asset_call;
                        asset_call.Connect(export_url);
                        asset_call.CreateCall("StoreItem");
                        asset_call.AddMember("account", request->account_);
                        asset_call.AddMember("UserServer", request->authserver_);
                        asset_call.AddMember("sessionhash", sessionhash);
                        asset_call.AddMember("itemname", name);
                        asset_call.AddMember("hashcode", asset->second.hash_);
                     
                        AvatarModule::LogDebug("Exporting asset " + name + " hash " + asset->second.hash_ + " datasize " + ToString<int>(asset->second.data_.size()));
                        
                        XMLRPC_VectorAppendBase64(asset_call.GetXMLRPCCall()->GetParamList(), "binaries", (const char*)(&asset->second.data_[0]), asset->second.data_.size());
                        
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
                        AvatarModule::LogInfo("Storage requested unknown missing asset " + name);
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
                AvatarModule::LogInfo("Re-exporting avatar xml");
                
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
                
                // Re-export avatar after exporting assets
                XmlRpcEpi call;
                call.Connect(export_url);
                call.CreateCall("StoreAvatarHash");
                call.AddMember("account", request->account_);
                call.AddMember("UserServer", request->authserver_);
                call.AddMember("sessionhash", sessionhash);
                call.AddMember("generic xml", request->avatar_xml_);
                
                ExportAssetMap::const_iterator i = request->assets_.begin();
                while (i != request->assets_.end())
                {
                    call.AddMember(i->second.hash_.c_str(), i->first);
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
    
    void ExportAsset::CalculateHash()
    {
        Poco::SHA1Engine sha1_engine;
        sha1_engine.update(&data_[0], data_.size());
        const Poco::DigestEngine::Digest& digest = sha1_engine.digest();

        QByteArray bytes((const char*)&digest[0], digest.size());
        QByteArray encoded = bytes.toBase64();
        hash_ = std::string(encoded.constData());
    }
}
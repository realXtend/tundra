// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "RexAssetMetadata.h"
#include "TinyJson/TinyJson.h"

#include <Poco/Base64Encoder.h>
#include <Poco/StreamCopier.h>

#include <QString>

namespace Asset
{
    RexAssetMetadata::RexAssetMetadata(): id_(""), name_(""), description_(""), creation_date_(time(0))
    {
    }

    const std::string& RexAssetMetadata::GetId() const
    {
        return id_;
    }

    const std::string& RexAssetMetadata::GetName() const
    {
        return name_;
    }

    const std::string& RexAssetMetadata::GetDescription() const
    {
        return description_;
    }

    time_t RexAssetMetadata::GetCreationDate() const
    {
        return creation_date_;
    }

    std::string RexAssetMetadata::GetCreationDateString() const
    {
        QString str = asctime(localtime(&creation_date_));
        // remove \n from the end
        str.remove(QChar('\n'), Qt::CaseInsensitive);
        return str.toStdString();
    }

    const std::string& RexAssetMetadata::GetContentType() const
    {
        return content_type_;
    }

    std::string RexAssetMetadata::GetHashSHA1() const
    {
        std::stringstream ss;
        for(size_t i = 0; i < 20; ++i)
            ss << std::hex << std::setw(2) << std::setfill('0') << (unsigned)hash_sha1_[i];
        return ss.str();
    }

    bool RexAssetMetadata::IsTemporary() const
    {
        return temporary_;
    }

    const Foundation::MethodMap& RexAssetMetadata::GetMethods() const
    {
        return methods_;
    }

    void RexAssetMetadata::DesesrializeFromJSON(std::string data)
    {
        json::grammar<char>::variant var = json::parse(data.begin(), data.end());

        if (var->type() != typeid(json::grammar<char>::object))
        {
            // PARSE ERROR: Root node must be object
            return;
        }

        json::grammar<char>::object const& obj = boost::any_cast<json::grammar<char>::object>(*var);
        for (json::grammar<char>::object::const_iterator it = obj.begin(); it != obj.end(); ++it)
        {
            std::string name = it->first;
            if (name == "id")
            {
                if (it->second->type() != typeid(std::string))
                    continue;

                id_ = boost::any_cast<std::string>(*it->second);
            }

            if (name == "name")
            {
                if (it->second->type() != typeid(std::string))
                    continue;

                name_ = boost::any_cast<std::string>(*it->second);
            }

            if (name == "description")
            {
                if (it->second->type() != typeid(std::string))
                    continue;

                description_ = boost::any_cast<std::string>(*it->second);
            }

            if (name == "creation_date")
            {
                if (it->second->type() != typeid(std::string))
                    continue;

                std::string date = boost::any_cast<std::string>(*it->second);
                //\todo parse date
            }

            if (name == "type")
            {
                if (it->second->type() != typeid(std::string))
                    continue;

                content_type_ = boost::any_cast<std::string>(*it->second);
            }

            if (name == "sha1")
            {
                if (it->second->type() != typeid(json::grammar<char>::array))
                    continue;

                json::grammar<char>::array const& sha1_bytes = boost::any_cast<json::grammar<char>::array>(*it->second);
                int p = 0;
                for (json::grammar<char>::array::const_iterator i = sha1_bytes.begin(); i != sha1_bytes.end(); ++i)
                {
                    if (i->get()->type() != typeid(int))
                        continue;

                    int b = boost::any_cast<int>(*i->get());
                    hash_sha1_[p++] = b;
                    if (p == 20)
                        break;
                }
            }

            if (name == "temporary")
            {
                if (it->second->type() != typeid(bool))
                    continue;

                temporary_ = boost::any_cast<bool>(*it->second);
            }

            if (name == "methods")
            {
                if (it->second->type() != typeid(json::grammar<char>::object))
                    continue;

                json::grammar<char>::object const& methods = boost::any_cast<json::grammar<char>::object>(*it->second);
                for (json::grammar<char>::object::const_iterator i = methods.begin(); i != methods.end(); ++i)
                {
                    if (i->second->type() != typeid(std::string))
                        continue;

                    std::string method_name = i->first;
                    std::string method_uri = boost::any_cast<std::string>(*i->second);
                    methods_.insert(std::pair<std::string, std::string>(method_name, method_uri));
                }
            }
        }
    }
}

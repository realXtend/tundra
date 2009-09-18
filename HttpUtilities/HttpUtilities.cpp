// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "HttpUtilities.h"

#include "Poco/URI.h"

namespace HttpUtilities
{
    std::string GetHostFromUrl(const std::string& url)
    {
        try
        {
            // Get the avatar storage host address
            Poco::URI uri(url);
            uri.setPath("");
            uri.setQuery("");
            return uri.toString();
        }
        catch (Poco::Exception e)
        {
            return std::string();
        }
    }
}

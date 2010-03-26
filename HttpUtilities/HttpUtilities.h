// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_HttpUtilities_HttpUtilities_h__
#define incl_HttpUtilities_HttpUtilities_h__

namespace HttpUtilities
{
    //! Returns url without path (protocol+host+port) Returns empty if illegal url
    /*! \param url Url to process
     */
    std::string GetHostFromUrl(const std::string& url);
    
    //! Global initialize of http services (Curl initialize)
    void InitializeHttp();
    
    //! Global shutdown of http services (Curl cleanup)
    void UninitializeHttp();
}

#endif
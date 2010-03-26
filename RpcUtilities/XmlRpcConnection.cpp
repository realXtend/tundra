// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "XmlRpcException.h"
#include "XmlRpcConnection.h"
#include "Poco/URI.h"
#include "boost/lexical_cast.hpp"

#include "HttpRequest.h"

XmlRpcConnection::XmlRpcConnection(const std::string& url)
{
    SetServer(url);
}

XmlRpcConnection::XmlRpcConnection(const std::string& address, const std::string& port)
{
    SetServerAddress(address, port);
}

void XmlRpcConnection::SetServerAddress(const std::string& address, const std::string& port) 
{ 
    std::string address_copy = address;
    if (address_copy.find("://") == std::string::npos)
        address_copy = "http://" + address_copy;
        
    Poco::URI uri = Poco::URI(address_copy);
    uri.setPort( boost::lexical_cast<int>(port) );
    
    strUrl_ = uri.toString();
}

XMLRPC_REQUEST XmlRpcConnection::Send(const char* data)
{
    HttpUtilities::HttpRequest request;
    request.SetUrl(strUrl_);
    request.SetRequestData("text/xml", data);
    request.SetMethod(HttpUtilities::HttpRequest::Post);
    request.Perform();
    
    const std::vector<u8> response_data = request.GetResponseData();
    
    if (!request.GetSuccess())
        throw XmlRpcException(std::string("XmlRpcEpi exception in XmlRpcConnection::Send() " + request.GetReason()));

    if (response_data.size() == 0)
        throw XmlRpcException(std::string("XmlRpcEpi exception in XmlRpcConnection::Send() response data size was zero: "));			
    
    // Convert the XML string to a XMLRPC reply structure.
    return XMLRPC_REQUEST_FromXML((const char*)&response_data[0], (int)(response_data.size()), 0);
}

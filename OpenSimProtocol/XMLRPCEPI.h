// For conditions of distribution and use, see copyright notice in license.txt
#ifndef incl_OpenSimProtocolModule_XMLRPCEPI_h
#define incl_OpenSimProtocolModule_XMLRPCEPI_h

#include "XMLRPCException.h"

#include <string>

class XMLRPCConnection;
class XMLRPCCall;

/**
 * This class purpose is to be easy interface for XMLRPC-epi function calls.
 * 
 * @code 
 *  // Normal call data flow, note if problem occures XMLRPCException is throwed
 *  
 *  XMLRPCEPI* myOb = new XMLRPCEPI();
 *  std::string address = "127.0.0.1";
 *  std::string port = "9000";
 *  // Creates connection which will used to transmite xmlrpc call. 
 *  myOb->Connect(address, port);
 *  std::string myMethod = "my_cool_method";
 *  // Creates a new xmlrpc call which method type is myMethod. Note old call data is destroyed. 
 *  myOb->CreateCall(myMethod);
 *  // Add value into xmlrpc call. 
 *  int value = 666;
 *  myOb->AddMember("attribute", value);
 *  // Send it to server 
 *  myOb->Send();
 *  // Get reply data
 *  std::string strReply = myOb->GetReply<std::string>("hyshKey");
 *  delete myOb;
 * @endcode
 */

class XMLRPCEPI
{

public:

    XMLRPCEPI();
    XMLRPCEPI(const std::string& method);
    XMLRPCEPI(const std::string& method, const std::string& address, const std::string& port);
    virtual ~XMLRPCEPI();

    /**
     * Initialises connection to given address. 
     * 
     * @param address is server address without port number
     * @param port is a server port which where calls are send. 
     * @return true if initialisation was successful. 
     **/
    
    void Connect(const std::string& address, const std::string& port);
    
    /**
     * Creates new call. Removes old call method from memory. 
     * @param method is new xmlrpc request method name. if method is empty call will be created with last saved method name 
     * see @p SetCallMethod
     */
    
    void CreateCall(const std::string& method = "");

    /**
     * Sends build xmlrpc-call through connection. All data which was added into xmlrpc call are held in memory until class 
     * user either destroys this object or calls @p CreateCall() method. 
     *
     * @note Connect method must be called at least once before any calls can be send (or special constructor must be used to construct this object)
     * 
     * @throws PocoXMLRPCException if message cannot be send or problem occures. 
     */
    
    void Send();

    /**
     * Sets a new call method name. 
     * @param method is new xmlrpc request method name. 
     */
    void SetCallMethod(const std::string& method) { strCallMethod_ = method;}
    std::string GetCallMethod() const { return strCallMethod_; }
    
     /**
     * Clears old call method data from memory. This method will remove all members which were saved using 
     * @p AddMember() and @p AddStringToArray() method. 
     */
    void ClearCall();   


    /// Adds a given string to a given name array.
    void AddStringToArray(const std::string& name, const char *sstr);

	template <typename T> T GetReply(const char* name) const;
    
    /// Adds a "name=value" parameter to the function call.
	template <typename T> void AddMember(const char* name, const T& value);

private:
    
    template <typename T> void Add(const char* name, const T& value);
	template <> void Add(const char* name, const int& value);
	template <> void Add(const char* name, const std::string& value);

    std::string strCallMethod_;
    XMLRPCConnection* pConnection_;
    XMLRPCCall* pCall_;

	
};

#include "XMLRPCEPI-templates.h"

#endif
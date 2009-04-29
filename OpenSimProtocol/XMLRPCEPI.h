// For conditions of distribution and use, see copyright notice in license.txt
#ifndef incl_OpenSimProtocolModule_XMLRPCEPI_h
#define incl_OpenSimProtocolModule_XMLRPCEPI_h

#include "XMLRPCException.h"

#include <string>

class XMLRPCConnection;
class XMLRPCCall;

/**
 * This class purpose is to be easy interface for XMLRPC-epi function calls. You only need to include this 
 * file and use this class to build nice custom XMLRPC-epi calls. 
 *
 * @note This class does not have custom copy-constructor or assign operator so use it carefully.
 * 
 * @code 
 *  // Normal call data flow, note if problem occures XMLRPCException is throwed
 *  
 *  XMLRPCEPI* myOb = new XMLRPCEPI;
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
    ~XMLRPCEPI();

    /**
     * Initialises connection to given address. 
     * 
     * @param address is server address without port number
     * @param port is a server port where calls are send. 
     * @throw XMLRPCException if problem arises.
     * 
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
     * @throw XMLRPCException if message cannot be send or problem occures. 
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
     * @throw XMLRPCException if call method is empty string.
     *
     */
    void ClearCall();   


    /**
     * Adds a given string to a given name array.
     * @param name is name of array where string is added.
     * @param sstr is string which will be added.
     */
    void AddStringToArray(const std::string& name, const char *sstr);

    /**
     * Returns value from xmlprc call reply, which is linked into given param name. 
     * @throw XMLRPCException if there does not exist given key or value conversion was invalid.
     *
     * @param name is name of key. 
     * 
     * @code
     *  int myKey = ob->GetReply<int>("hashKey");
     * @endcode
     */
	template <typename T> T GetReply(const char* name) const;
    
     /**
      * Adds a "name=value" parameter to the function call.
      * @throw XMLRPCException if there exist any call object. Check out @p CreateCall in that case.  
      * 
      * @param name is name of member. 
      * @param value is a value which will be added into member.
      * 
      */
	template <typename T> void AddMember(const char* name, const T& value);

private:
    // Default implementation does not do anything. 
    template <typename T> void Add(const char* name, const T& value);
   
    std::string strCallMethod_;
    XMLRPCConnection* pConnection_;
    XMLRPCCall* pCall_;

	
};

#include "XMLRPCEPI-templates.h"

#endif

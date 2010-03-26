// For conditions of distribution and use, see copyright notice in license.txt
#ifndef incl_RpcUtilities_XmlRpcEpi_h
#define incl_RpcUtilities_XmlRpcEpi_h

#include "XmlRpcException.h"
#include <string>

class XmlRpcConnection;
class XmlRpcCall;

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

	class XmlRpcEpi
	{

	public:

		XmlRpcEpi();
		explicit XmlRpcEpi(const std::string& method);
		XmlRpcEpi(const std::string& method, const std::string& address, const std::string& port);
		XmlRpcEpi(const std::string& method, const std::string& url);
		~XmlRpcEpi();

		/** Initialises connection to given address. This doesn't actually build up a network connection,
			but instead the address and port are stored for later use when Send() is called. You have to
			call this function at least once to set the address before calling Send() (if you didn't initialize
			using the ctor that takes in address and port). You may call this function multiple times before
			calling Send() to change the address where the connection is made.
			@param address The server hostname (IP address or a domain name), without the port number.
			@param port The port on the server where the connection is made.
			@throw XMLRPCException if problem arises. */
		void Connect(const std::string& address, const std::string& port);
	    
	    /** Initialises connection to given url
            @param url Full url to connect to
	     */
	    void Connect(const std::string& url);
	    
		/**
		 * Creates new call. Removes old call method from memory. 
		 * @param method is new xmlrpc request method name. if method is empty call will be created with last saved method name 
		 * see @p SetCallMethod
		 */    
		void CreateCall(const std::string& method = "");

		/** You can use this method to gain access to the internal XMLRPCCall structure, e.g. if you want to use the C API
			to manage the parameters in the request and reply structs.
			@return The XMLRPCCall object that corresponds to the current call, or 0 if no call has been created. */
		XmlRpcCall *GetXMLRPCCall();

		/** Sends the built xmlrpc-call through connection. All data which was added into the call are held in memory until the 
			class user either destroys this object or calls @p CreateCall() method. 
			@note Connect method must be called at least once before any calls can be send (or special constructor must be used to construct this object)
			@throw XMLRPCException if message cannot be send or problem occures. */
		void Send();

		/**
		 * Sets a new call method name. 
		 * @param method is new xmlrpc request method name. 
		 */
		void SetCallMethod(const std::string& method) { callMethod_ = method;}
		std::string GetCallMethod() const { return callMethod_; }
	    
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

        //! Returns if call reply contains a value. May be of any type. Does not throw exceptions
        bool HasReply(const char* name) const;

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
		 * Returns vector of values from xmlprc call reply, which is linked into given param name. 
		 * @throw XMLRPCException if there does not exist given key or value conversion was invalid.
		 *
		 * @param name is name of key. 
		 * 
		 * @code
		 *  int myKey = ob->GetReply<int>("hashKey");
		 * @endcode
		 */
		template <typename T> std::vector<T> GetVectorReply(const char* name) const;
		
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
	   
		std::string callMethod_;
		XmlRpcConnection* connection_;
		XmlRpcCall* call_;
	};

	#include "XmlRpcEpiTemplates.h"

#endif

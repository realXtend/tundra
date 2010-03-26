// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RpcUtilities_XmlRpcException_h
#define incl_RpcUtilities_XmlRpcException_h

#include <cstdlib>
#include <cstring>
#include <string>

#include "CoreException.h"

	/**
	 * XMLRPCException is exception type which is throwed in cases if you use XMLRPC-epi interface and problem arises. 
	 * @see Exception
	 */
	class XmlRpcException : public std::exception
	{
	public:
		///\todo Add the originating XMLRPCEPI object.
		explicit XmlRpcException(const std::string &str)
		:msg_(str)
		{
		}

		virtual ~XmlRpcException () throw()
		{
		}
	    
		const char *what () const throw () { return msg_.c_str(); }

		/**
		 * Sets error message. 
		 * @param message is string which contains error message.
		 */
		void SetMessage(const std::string& message) { msg_ = message; }

	private:

		std::string msg_;
	};

#endif 

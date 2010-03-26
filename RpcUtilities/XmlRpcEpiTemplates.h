// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RpcUtilities_XmlRpcEpi_h
# error "Never use <XmlRpcEpiTemplates.h> directly; include <XmlRpcEpi.h> instead."
#endif

#include <xmlrpc.h>
#include "XmlRpcCall.h"
#include <boost/lexical_cast.hpp>


template <typename T> T XmlRpcEpi::GetReply(const char* name) const 
{
	assert(name && strlen(name) > 0);

	if (call_ == 0)
		throw XmlRpcException(std::string("XmlRpcEpi exception in GetReply() error: Call object is zero pointer"));

	// I'm probably not understanding the value hierarchy here.. samples use XMLRPC_VectorRewind(XMLRPC_RequestGetData(request))
	// but it seems to walk into the first element of the vector, after which GetValueWithID doesn't find the correct sibling. wtf?

	XMLRPC_VALUE result = XMLRPC_RequestGetData(call_->GetReply()); //XMLRPC_VectorRewind(XMLRPC_RequestGetData(request));
	assert(result);

	T value = T();

	if (!result)
		throw XmlRpcException(std::string("XmlRpcEpi exception in GetReply() error: XML reply did not contain any data!"));

	XMLRPC_VALUE resultValue = XMLRPC_VectorGetValueWithID(result, name);

	if (resultValue)
	{
		XMLRPC_VALUE_TYPE type = XMLRPC_GetValueType(resultValue);

		try
		{            
			switch (type)
			{
			case xmlrpc_none:
			{
				// None type 
				std::string strName(name);
				throw XmlRpcException(std::string("XmlRpcEpi exception in GetReply() error: XML reply contain NONE data! (Tried to retrieve reply by ID ") + strName);
				break; 
			}
			case xmlrpc_empty:
			{
				//Empty type
				std::string strName(name);
				throw XmlRpcException(std::string("XmlRpcEpi exception in GetReply() error: XML reply contain EMPTY data! (Tried to retrieve reply by ID ") + strName);
				break;
			}
			case xmlrpc_base64:
			{
				// base64 value, eg. binary data
				value = boost::lexical_cast<T>(XMLRPC_GetValueBase64(resultValue));
				break;
			}
			case xmlrpc_boolean:
			{
				// boolean
				value = boost::lexical_cast<T>(XMLRPC_GetValueBoolean(resultValue));
				break;
			}
			case xmlrpc_datetime:
			{
				//datetime [ISO8601 | time_t]
				value = boost::lexical_cast<T>(XMLRPC_GetValueDateTime(resultValue));
				break;
			}
			case xmlrpc_double:
			{
				//double / floating point
				value = boost::lexical_cast<T>(XMLRPC_GetValueDouble(resultValue));
				break;
			}
			case xmlrpc_int:
			{
				// Integer
				value = boost::lexical_cast<T>(XMLRPC_GetValueInt(resultValue));
				break;
			}
			case xmlrpc_string:
			{
				//string (const char* actually)
				value = boost::lexical_cast<T>(XMLRPC_GetValueString(resultValue));
				break;
			}
			case xmlrpc_vector:
			{
				//vector, aka list, array 
				std::string strName(name);
				throw XmlRpcException(std::string("XmlRpcEpi exception in GetReply() error: XML reply contain Vector data! (Tried to retrieve reply by ID ") + strName);
				//LogError("Error! XML reply contain vector type data! (Tried to retrieve int by ID " + ToString(name)+ ".");
				break;
			}

			default:
				break;
			}
		}
		catch (boost::bad_lexical_cast&)
		{
			std::string strName(name);
			throw XmlRpcException(std::string("XmlRpcEpi exception in GetReply() error: XML reply data was not converted to wanted type! (Tried to retrieve reply by ID ") + strName);
		}
	}
	else
	{
		std::string strName(name);
		throw XmlRpcException(std::string("XmlRpcEpi exception in GetReply() error: XML reply data was not found! (Tried to retrieve reply by ID ") + strName);
	}

	return value;
}

template <typename T> std::vector<T> XmlRpcEpi::GetVectorReply(const char* name) const 
{
	assert(name && strlen(name) > 0);

    std::vector<T> values;
    
	if (call_ == 0)
		throw XmlRpcException(std::string("XmlRpcEpi exception in GetReply() error: Call object is zero pointer"));

	// I'm probably not understanding the value hierarchy here.. samples use XMLRPC_VectorRewind(XMLRPC_RequestGetData(request))
	// but it seems to walk into the first element of the vector, after which GetValueWithID doesn't find the correct sibling. wtf?

	XMLRPC_VALUE result = XMLRPC_RequestGetData(call_->GetReply()); //XMLRPC_VectorRewind(XMLRPC_RequestGetData(request));
	assert(result);

	if (!result)
		throw XmlRpcException(std::string("XmlRpcEpi exception in GetReply() error: XML reply did not contain any data!"));

	XMLRPC_VALUE resultValue = XMLRPC_VectorGetValueWithID(result, name);

	if (resultValue)
	{
		XMLRPC_VALUE_TYPE type = XMLRPC_GetValueType(resultValue);

		try
		{
			switch (type)
			{
			case xmlrpc_vector:
			{
                XMLRPC_VALUE entry = XMLRPC_VectorRewind(resultValue);
                while (entry)
                {
                    const char* entry_str = XMLRPC_GetValueString(entry);
                    
                    if (entry_str)
                    {
                        T value = boost::lexical_cast<T>(std::string(entry_str));
                        values.push_back(value);
                    }
                    
                    entry = XMLRPC_VectorNext(resultValue);
                }
				break;
			}

			default:
			    throw XmlRpcException(std::string("XmlRpcEpi exception in GetReply() error: XML reply did not contain Vector data! (Tried to retrieve reply by ID ") + name);
				break;
			}
		}
		catch (boost::bad_lexical_cast&)
		{
			std::string strName(name);
			throw XmlRpcException(std::string("XmlRpcEpi exception in GetReply() error: XML reply data was not converted to wanted type! (Tried to retrieve reply by ID ") + strName);
		}
	}
	else
	{
		std::string strName(name);
		throw XmlRpcException(std::string("XmlRpcEpi exception in GetReply() error: XML reply data was not found! (Tried to retrieve reply by ID ") + strName);
	}

	return values;
}



template <typename T> void XmlRpcEpi::AddMember(const char* name, const T& value) 
{
	if (call_ == 0)
	{
	   std::string strName(name);
	   throw XmlRpcException(std::string("XMLRPCEPI exception in AddMember() error: Call object was zero pointer (Tried to add member ID ") + strName);
	}

	Add(name, value);
}

template <> inline void XmlRpcEpi::Add<int>(const char* name, const int& value)
{
	XMLRPC_VectorAppendInt(call_->GetParamList(), name, value);
}

template <> inline void XmlRpcEpi::Add<std::string>(const char* name, const std::string& value)
{
	XMLRPC_VectorAppendString(call_->GetParamList(), name, value.c_str(), 0);
}

template <typename T> void XmlRpcEpi::Add(const char* name, const T& value) 
{
	///\todo Is this deliberately unsupported or not written because of lack of time? -jj.
	/// If unsupported, undefine.
}
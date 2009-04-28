// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OpenSimProtocolModule_XMLRPCEPI_h
# error "Never use <XMLRPCEPI-templates.h> directly; include <XMLRPCEPI.h> instead."
#endif

#include <xmlrpc.h>
#include "XMLRPCCall.h"


template <typename T> T XMLRPCEPI::GetReply(const char* name) const 
{
	assert(name && strlen(name) > 0);
	
    if ( pCall_ == 0)
    {
        throw XMLRPCException(std::string("XMLRPCEPI exception in GetReply() error: Call object is zero pointer"));
    }

	// I'm probably not understanding the value hierarchy here.. samples use XMLRPC_VectorRewind(XMLRPC_RequestGetData(request))
	// but it seems to walk into the first element of the vector, after which GetValueWithID doesn't find the correct sibling. wtf?
	
	XMLRPC_VALUE result = XMLRPC_RequestGetData(pCall_->GetReply()); //XMLRPC_VectorRewind(XMLRPC_RequestGetData(request));
	assert(result);
	
	T value = T();

	if (!result)
	{
		throw XMLRPCException(std::string("XMLRPCEPI exception in GetReply() error: XML reply did not contain any data!"));
		
	}
	
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
					   throw XMLRPCException(std::string("XMLRPCEPI exception in GetReply() error: XML reply contain NONE data! (Tried to retrieve reply by ID ") + strName);
                       break; 
					}
            case xmlrpc_empty:
					{
						//Empty type
                        std::string strName(name);
                        throw XMLRPCException(std::string("XMLRPCEPI exception in GetReply() error: XML reply contain EMPTY data! (Tried to retrieve reply by ID ") + strName);
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
                        throw XMLRPCException(std::string("XMLRPCEPI exception in GetReply() error: XML reply contain Vector data! (Tried to retrieve reply by ID ") + strName);
					    //LogError("Error! XML reply contain vector type data! (Tried to retrieve int by ID " + Core::ToString(name)+ ".");
						break;
					}
				
				default:
					break;
			}
			
        }
        catch (boost::bad_lexical_cast&)
	 	{
            std::string strName(name);
            throw XMLRPCException(std::string("XMLRPCEPI exception in GetReply() error: XML reply data was not converted to wanted type! (Tried to retrieve reply by ID ") + strName);
		}
	        
	}
    else
    {
        std::string strName(name);
        throw XMLRPCException(std::string("XMLRPCEPI exception in GetReply() error: XML reply data was not found! (Tried to retrieve reply by ID ") + strName);
    }
	
	return value;

	
}

template <typename T> void XMLRPCEPI::AddMember(const char* name, const T& value) 
{
    if ( pCall_ != 0 )
        Add(name, value);
    else
    {
       std::string strName(name);
       throw XMLRPCException(std::string("XMLRPCEPI exception in AddMember() error: Call object was zero pointer (Tried to add member ID ") + strName);
    }
}

template <typename T> void XMLRPCEPI::Add(const char* name, const T& value) 
{}

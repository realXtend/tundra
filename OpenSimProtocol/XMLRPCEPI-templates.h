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
        // Throw...
        return T();
    }

	// I'm probably not understanding the value hierarchy here.. samples use XMLRPC_VectorRewind(XMLRPC_RequestGetData(request))
	// but it seems to walk into the first element of the vector, after which GetValueWithID doesn't find the correct sibling. wtf?
	
	XMLRPC_VALUE result = XMLRPC_RequestGetData(pCall_->GetReply()); //XMLRPC_VectorRewind(XMLRPC_RequestGetData(request));
	assert(result);
	
	T value;

	if (!result)
	{
		
		//LogError("Error! XML reply did not contain any data! (Tried to retrieve int by ID " + Core::ToString(name)+ ".");
		return value;
	}
	
	XMLRPC_VALUE resultValue = XMLRPC_VectorGetValueWithID(result, name);
	

	if (resultValue)
	{
		XMLRPC_VALUE_TYPE type = XMLRPC_GetValueType(resultValue);
		
        try
		{
            
			switch (type)
			{
				case 0:
					{
						// None type 
						//LogError("Error! XML reply containded None type! (Tried to retrieve int by ID " + Core::ToString(name)+ ".");
                        
                        //std::string strName(name);
                        //PocoXMLRPCException ex(std::string("Error! XML reply contain None type! (Tried to retrieve reply by ID ") + strName);
                        //throw ex;
						break; 
					}
				case 1: 
					{
						//Empty type
						//LogError("Error! XML reply containded empty type! (Tried to retrieve int by ID " + Core::ToString(name)+ ".");
						break;
					}
				case 2:
					{
						// base64 value, eg. binary data
						value = boost::lexical_cast<T>(XMLRPC_GetValueBase64(resultValue));
						break;
					}
				case 3:
					{
						// boolean
						value = boost::lexical_cast<T>(XMLRPC_GetValueBoolean(resultValue));
						break;
					}
				case 4: 
					{
						//datetime [ISO8601 | time_t]
						value = boost::lexical_cast<T>(XMLRPC_GetValueDateTime(resultValue));
						break;
					}
				case 5:
					{
						//double / floating point
						value = boost::lexical_cast<T>(XMLRPC_GetValueDouble(resultValue));
						break;
					}
				case 6:
					{
						// Integer
						value = boost::lexical_cast<T>(XMLRPC_GetValueInt(resultValue));
						break;
					}
				case 7:
					{
						//string (const char* actually)
						value = boost::lexical_cast<T>(XMLRPC_GetValueString(resultValue));
						break;
					}
				case 8:
					{
						//vector, aka list, array 
					    //LogError("Error! XML reply contain vector type data! (Tried to retrieve int by ID " + Core::ToString(name)+ ".");
						break;
					}
				
				default:
					break;
			}
			
        }
        catch (boost::bad_lexical_cast&)
	 	{
			//LogError("Error! XML reply did not managed to cast reply data to wanted data type (Tried to retrieve data by ID " + Core::ToString(name)+ ".");
		}
	        
	}
	
	return value;

	
}

template <typename T> void XMLRPCEPI::AddMember(const char* name, const T& value) 
{
    if ( pCall_ != 0 )
        Add(name, value);
    else
    {
        // throw exception..
    }
}


template <> void XMLRPCEPI::Add<int>(const char* name, const int& value) 
{
    XMLRPC_VectorAppendInt(pCall_->GetParamList(), name, value);
}

template <> void XMLRPCEPI::Add<std::string>(const char* name, const std::string& value) 
{
    XMLRPC_VectorAppendString(pCall_->GetParamList(), name, value.c_str(), 0);
}

template <typename T> void XMLRPCEPI::Add(const char* name, const T& value) 
{}

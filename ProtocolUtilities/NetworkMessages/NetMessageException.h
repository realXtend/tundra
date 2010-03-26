// For conditions of distribution and use, see copyright notice in license.txt
#ifndef incl_Protocol_NetMessageException_h
#define incl_Protocol_NetMessageException_h

#include <cstdlib>
#include <cstring>
#include <string>

///\todo C4996 warnings caused by strcpy disabled for now. Fix usage of strcpy in CoreException.h.
#pragma warning( push )
#pragma warning( disable : 4996 )
#include "CoreException.h"
#pragma warning( pop )

/**
 * NetMessageException is exception type which is throwed in cases if you use NetMessage interface and a problem arises.
 * @see Exception
 */
 
class NetMessageException : public Exception
{
public:
    NetMessageException() : Exception(), strMessage_(""), type_(ET_None) {}
    virtual ~NetMessageException() throw () {} 
    
    /// Enumeration of the exception types.
    enum ExceptionType 
    {
        ET_None = 0,
        ET_VariableTypeMismatch,
        ET_BlockInstanceCountNotRead,
        ET_BlockInstanceCountAlreadyRead,
        ET_InvalidVariableName,
        ET_EnumCount
    };

    /**
     * Override constructor. 
     * @param message is error message. 
     */
    explicit NetMessageException(const std::string& message) : Exception(), strMessage_(message), type_(ET_None) {}

    /**
     * Override constructor. 
     * @param type Exception type enum. 
     */
    explicit NetMessageException(ExceptionType type) : Exception(), type_(type) {}
    
    /**
     * Returns error message for caller. 
     * @return message which contains error. 
     */
    std::string GetMessage() const { return strMessage_; }
    
    /**
     * Sets error message. 
     * @param message is string which contains error message.
     */
    void SetMessage(const std::string& message) { strMessage_ = message; }

    /**
     * Sets error message. 
     * @return Exception type enum.
     */
    const ExceptionType GetType() const  { return type_; }

    /** Utility function for converting the connection state enum to string.
     * @param The connection state enum.
     * @return The login state as a string.
     */
    const std::string &What() const
    {
        static const std::string exception_strings[ET_EnumCount] = {
            "None",
            "Tried to read wrong variable type",
            "Current block is variable: use ReadCurrentBlockInstanceCount first in order to proceed",
            "This block's instance count is already read",
            "Invalid nonexisting variableName input to NetInMessage::SkipToFirstVariableByName"
            };

        return exception_strings[type_];
    }

private:
    /// Exception error message.
    std::string strMessage_;
    
    /// Exception type.
    ExceptionType type_;

};

#endif // incl_Protocol_NetMessageException_h

// For conditions of distribution and use, see copyright notice in license.txt
#include "StableHeaders.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <boost/cstdint.hpp>

#include "NetMessageList.h"
#include "CoreDefines.h"

using namespace std;
using boost::uint32_t;

namespace ProtocolUtilities
{

/// Indicates the state of the parsing process.
enum ParseState
{
    TopBraceOpen,
    PacketType,
    PacketDataBraceOpen,
    PacketDataBraceOpenOrBraceClose,
    PacketBlockName,
    PacketBlockDataOrBraceClose
};

/// Identifies the priority level of a network message.
enum NetPriorityLevel
{
    NetPriorityInvalid = 0,
    NetPriorityHigh,
    NetPriorityMed,
    NetPriorityLow,
    NetPriorityFixed
};

/// Parses one line of the template file.
/// @param file A C string of the contents of the file in memory.
int ParseFirstLine(const char *file, char *dst, int dstLen);

/// Loads the given file into a C string. Call delete[] when you're done with it.
char *LoadFileToString(const char *filename);

/// Cuts whitespaces off the string in-place. Replaces them with '\0'.
size_t TrimTrailingWhitespaces(char *str);

/// Cuts single-line comments off the line, in-place. Replaces them with '\0'.
size_t RemoveComments(char *line);

NetMessageList::NetMessageList(const char *filename)
{
    ParseMessageListFromFile(filename);
}

NetMessageList::~NetMessageList()
{

}

/// @return Line length, or -1 if at end.
int ParseFirstLine(const char *file, char *dst, int dstLen)
{
    if (dstLen == 0)
        return -1;

    if (!file)
    {
        *dst = 0;
        return -1;
    }

    if (*file == '\0')
    {
        *dst = 0;
        return -1;
    }

    int trimmedLength = 0;
    // Trim whitespace and invalid characters from line start.
    while(((unsigned char)*file <= 0x20 || (unsigned char)*file >= 0x80) && *file != '\0')
    {
        ++trimmedLength;
        ++file;
    }

    if (*file == '\0')
    {
        *dst = 0;
        return -1;
    }

    // Go through the line data.
    int length = 0;
    while(*file != '\0' && *file != '\n' && (unsigned char)*file <= 0x7f)
    {
        *dst = *file;
        ++length;
        ++file;
        ++dst;
        if (length+1 >= dstLen)
            break;
    }
    *dst = '\0';

    return length + trimmedLength;
}

///\todo Memory management.
char *LoadFileToString(const char *filename)
{
    unsigned long len;
    char *data;
    std::fstream filestr(filename, fstream::in);
    
    if (!filestr)
        return 0;
    
    filestr.seekg (0, ios::end);
    len = filestr.tellg();
    filestr.seekg (ios::beg);
    data = new char[len+1];
    memset(data, 0, len+1);
    filestr.read(data, len);
    data[len] = '\0';
    filestr.close();

    return data;
}


/// @return Length of the new string.
size_t TrimTrailingWhitespaces(char *str)
{    
    size_t len = strlen(str);
    char *s = &str[len-1];

    while((unsigned char)*s <= 0x20 && s >= str)
    {
        *s = '\0';
        --s;
        --len;
    }

    return len;
}

/// @return Length of the new string.
size_t RemoveComments(char *line)
{
    char *occur = strstr(line, "//");

    if (!occur)
        return strlen(line);
    else
    {
        *occur = '\0';
        return TrimTrailingWhitespaces(line);
    }
}

/// @return Variable type as an integer.
NetVariableType StrToVariableType(const char *str)
{
    const char *data[] = { "Invalid", "U8", "U16", "U32", "U64", "S8", "S16", "S32", "S64", "F32", "F64", "LLVector3", "LLVector3d", "LLVector4",
                           "LLQuaternion", "LLUUID", "BOOL", "IPADDR", "IPPORT", "Fixed", "Variable" };

    for(int i = 0; i < NUMELEMS(data); ++i)

#ifdef _MSC_VER
        if (!_strcmpi(data[i], str))
#else
        if (!strcasecmp(data[i], str))
#endif
            return (NetVariableType)i;

    return NetVarInvalid;
}

/// @return Block type as an integer
NetMessageBlockType StrToBlockType(const char *str)
{
    if (!strcmp(str, "Single")) return NetBlockSingle;
    if (!strcmp(str, "Multiple")) return NetBlockMultiple;
    if (!strcmp(str, "Variable")) return NetBlockVariable;
    return NetBlockInvalid;
}

/// @return Priority level as an string
const char *PriorityLevelToStr(NetPriorityLevel p)
{
    if (p == NetPriorityFixed) return "Fixed";
    if (p == NetPriorityHigh) return "High";
    if (p == NetPriorityMed) return "Med";
    if (p == NetPriorityLow) return "Low";
    return "Invalid";
}

/// @return Priority level as an integer
NetPriorityLevel StrToPriorityLevel(const char *p)
{
    if (!strcmp(p, "High")) return NetPriorityHigh;
    if (!strcmp(p, "Medium")) return NetPriorityMed;
    if (!strcmp(p, "Low")) return NetPriorityLow;
    if (!strcmp(p, "Fixed")) return NetPriorityFixed;
    return NetPriorityInvalid;
}

/// @return Priority number as an integer
uint32_t StrToPriorityNumber(const char *p)
{
    if (!strncmp(p, "0x", 2))
    {
        p += 2;
        stringstream str(stringstream::in | stringstream::out);
        str << p;
        unsigned long i;
        str >> std::hex >> i;
        return i;
    }

    stringstream str;
    str << p;
    unsigned long i;
    str >> i;
    return i;
}

/// @return Priority number as an integer
NetTrustLevel StrToTrustLevel(const char *str)
{
    if (!strcmp(str, "NotTrusted")) return NetNotTrusted;
    if (!strcmp(str, "Trusted")) return NetTrusted;
    return NetTrustLevelInvalid;
}

/// @return Priority number as a boolean
NetEncoding StrToEncoding(const char *str)
{
    if (!strcmp(str, "Unencoded")) return NetUnencoded;
    if (!strcmp(str, "Zerocoded")) return NetZeroEncoded;
    return NetEncodingInvalid;
}

static NetMsgID PriorityAndMsgNumberToMsgID(NetPriorityLevel priority, uint32_t number)
{
    switch(priority)
    {
    case NetPriorityHigh:
    case NetPriorityFixed:
        return (NetMsgID)number;
        break;
    case NetPriorityMed:
        return number | 0xFF00;
    case NetPriorityLow:
        return number | 0xFFFF0000;
    default:
        assert(false); ///\todo Error propagation.
        return 0;
    }
}

/// @return Parsing state as a string
const char *ParseStateToString(ParseState s)
{
    const char *d[] = { "TopBraceOpen", "PacketType", "PacketDataBraceOpen", "PacketDataBraceOpenOrBraceClose",
                        "PacketBlockName", "PacketBlockDataOrBraceClose" };
    return d[s];
}

const NetMessageInfo *NetMessageList::GetMessageInfoByID(NetMsgID id) const
{
    NetworkMessageMap::const_iterator iter = messages.find(id);
    if (iter != messages.end())
        return &iter->second;
    else
        return 0;
}

void NetMessageList::ParseMessageListFromFile(const char *filename)
{
    char *file = LoadFileToString(filename);
    char *data = file;

    const int maxLen = 512;
    char line[maxLen];

    ParseState parseState = TopBraceOpen;
    NetMessageInfo *curMsg = 0;
    NetMessageBlock *curBlock = 0;

    for(;;)
    {
        // Read next line.    
        size_t length = ParseFirstLine(data, line, 510);
        if (length == -1) // If an error, quit.
            break;
        data += length;
        length = RemoveComments(line);
        if (length <= 0) // If line empty, go to next.
            continue;
        length = TrimTrailingWhitespaces(line);
        if (length <= 0) // If line empty, go to next.
            continue;

        switch(parseState)
        {
        case TopBraceOpen:
            if (!strcmp(line, "{"))
                parseState = PacketType;
            break;
        case PacketType:
            {
                char packetName[maxLen];
                char priorityType[maxLen];
                char priorityNumber[maxLen];
                char trusted[maxLen];
                char zeroCoded[maxLen];
                
                sscanf(line, "%s %s %s %s %s", packetName, priorityType, priorityNumber, trusted, zeroCoded);
                NetMessageInfo msgInfo;
                msgInfo.name = packetName;
                NetPriorityLevel priorityLevel = StrToPriorityLevel(priorityType);
                uint32_t number = StrToPriorityNumber(priorityNumber);
/*                if (msgInfo.priority == NetPriorityFixed)
                {
                    msgInfo.priority = NetPriorityLow;
                    msgInfo.priorityNumber &= 0xFFFF;
                }*/
                msgInfo.id = PriorityAndMsgNumberToMsgID(priorityLevel, number);
                msgInfo.trustLevel = StrToTrustLevel(trusted);
                msgInfo.encoding = StrToEncoding(zeroCoded);
                messages[msgInfo.id] = msgInfo;
                curMsg = &messages[msgInfo.id];
                curBlock = 0;

                parseState = PacketDataBraceOpenOrBraceClose;
            }
            break;
        case PacketDataBraceOpenOrBraceClose:
            if (!strcmp(line, "}"))
                parseState = TopBraceOpen;
            // fall through.
        case PacketDataBraceOpen:
            if (!strcmp(line, "{"))
                parseState = PacketBlockName;
            break;
        case PacketBlockName:
            {            
                char blockName[maxLen];
                char blockType[maxLen];
                int blockRepeatCount = 0;
                sscanf(line, "%s %s %d", blockName, blockType, &blockRepeatCount);
                NetMessageBlock msgBlock;
                msgBlock.name = blockName;

                msgBlock.type = StrToBlockType(blockType);
                if (msgBlock.type == NetBlockSingle)
                    blockRepeatCount = 1;
                msgBlock.repeatCount = blockRepeatCount;
                curMsg->blocks.push_back(msgBlock);
                curBlock = &curMsg->blocks.back();
                parseState = PacketBlockDataOrBraceClose;
            }
            break;
        case PacketBlockDataOrBraceClose:
            {
                if (line[0] == '{')
                {
                    char varName[maxLen];
                    char varType[maxLen];
                    int varSize = 0;
                    sscanf(&line[1], "%s %s %d", varName, varType, &varSize);
                    NetMessageVariable msgVar;
                    msgVar.name = varName;

                    msgVar.type = StrToVariableType(varType);
                    if (msgVar.type == NetVarBufferByte)
                        msgVar.count = 0;
                    else
                        msgVar.count = varSize;

                    // Is this a 2-byte encoded variable sized parameter?
                    if (msgVar.type == NetVarBufferByte && varSize == 2)
                        msgVar.type = NetVarBuffer2Bytes;

                    curBlock->variables.push_back(msgVar);
                }
                else if (line[0] == '}')
                    parseState = PacketDataBraceOpenOrBraceClose;
            }
            break;
        } // ~switch
    } // ~for
    delete[] file;
}

static bool NetMessageInfoCmp(const NetMessageInfo &a, const NetMessageInfo &b)
{
    return a.id < b.id;
}

void NetMessageList::GenerateHeaderFile(const char *filename) const
{
    ofstream out(filename);

    out << "/* This file defines constants for all the messages used in the protocol. The values" << endl
        << "match to the IDs of the messages, and are used for identifying different kinds of " << endl
        << "packets. This file is automatically generated from the message template file, so no " << endl
        << "point modifying it here. */" << endl
        << endl
        << "#ifndef RexProtocolMsgIDs" << endl
        << "#define RexProtocolMsgIDs" << endl
        << endl;

    std::vector<NetMessageInfo> msgs;
    for(NetworkMessageMap::const_iterator iter = messages.begin(); iter != messages.end(); ++iter)
    {
        const NetMessageInfo &msg = iter->second;
        msgs.push_back(msg);
    }
    std::sort(msgs.begin(), msgs.end(), NetMessageInfoCmp);

    for(std::vector<NetMessageInfo>::const_iterator iter = msgs.begin(); iter != msgs.end(); ++iter)
    {
        const NetMessageInfo &msg = *iter;
        out << "const NetMsgID RexNetMsg" << setw(40) << left << msg.name << " = " << "0x" << hex << msg.id << ";" << endl;
    }

    out << endl << "#endif" << endl;
}

}
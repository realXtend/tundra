// For conditions of distribution and use, see copyright notice in license.txt
#ifndef incl_ProtocolUtilities_NetMessage_h
#define incl_ProtocolUtilities_NetMessage_h

#include <string>
#include <vector>
//#include "RexTypes.h"

namespace ProtocolUtilities
{
    /// UDP message header flags.
    enum NetHeaderFlag
    {
        NetFlagAck = 0x10, ///< The message contains appended ACKs.
        NetFlagResent = 0x20, ///< This message is a resend, since the other end didn't ACK it in time.
        NetFlagReliable = 0x40, ///< This message needs to be ACKed.
        NetFlagZeroCode = 0x80  ///< This message is compressed by RLE-encoding zeroes.
    };

    /// Identifies the type of a variable inside a network message block.
    /// \ingroup OpenSimProtocolClient
    enum NetVariableType
    {
        NetVarInvalid = 0,
        NetVarU8,
        NetVarU16,
        NetVarU32,
        NetVarU64,
        NetVarS8,
        NetVarS16,
        NetVarS32,
        NetVarS64,
        NetVarF32,
        NetVarF64,
        NetVarVector3,
        NetVarVector3d,
        NetVarVector4,
        NetVarQuaternion,
        NetVarUUID,
        NetVarBOOL,
        NetVarIPADDR,
        NetVarIPPORT,
        NetVarFixed,
        NetVarBufferByte,   ///< A variable-length buffer where the length is encoded by one byte. (<= 255 bytes in size)
        NetVarBuffer2Bytes, ///< A variable-length buffer where the length is encoded by two bytes. (<= 65535 bytes in size)
        NetVarBuffer4Bytes, ///< A variable-length buffer where the length is encoded by four bytes. (<= 4GB bytes in size)
        NetVarNone, ///< Tells the message generator that no more variables are expected, used as a sentinel.
        NetVariableTypeEnumCount, ///< The number of valid variable types.
    };

    /// Identifies the size of variables inside a network message block. The indices match the ones above. ///\todo Replace with a type trait / utility function.
    const size_t NetVariableSizes[] = { 0, 1, 2, 4, 8, 1, 2, 4, 8, 4, 8, 12, 24, 16, 12, 16, 1, 4, 2, 0, 1, 2, 4, 0 };

    /// Identifies the trust level of a message.
    enum NetTrustLevel
    {
        NetTrustLevelInvalid = 0,
        NetNotTrusted,
        NetTrusted,
        NetTrustLevelEnumCount,
    };

    /// Identifies the encoding of a message.
    enum NetEncoding
    {
        NetEncodingInvalid = 0,
        NetUnencoded,
        NetZeroEncoded,
        NetEncodingEnumCount,
    };

    /// Describes a variable that is present in a message block.
    /// \ingroup OpenSimProtocolClient
    struct NetMessageVariable
    {
        std::string name;
        NetVariableType type;

        ///< The number of times this variable occurs in the stream, or the length of the buffer in bytes, if type == NetVarBufferXX.
        size_t count; 
    };

    /// Identifies the type of a message block inside a network message.
    /// \ingroup OpenSimProtocolClient
    enum NetMessageBlockType
    {
        NetBlockInvalid = 0,
        NetBlockSingle,
        NetBlockMultiple,
        NetBlockVariable 
    };

    /// Network messages consist of message blocks. This structure describes a single block.
    /// \ingroup OpenSimProtocolClient
    struct NetMessageBlock
    {
        std::string name;
        NetMessageBlockType type;
        /// How many times this block is repeated in the message.
        size_t repeatCount;
        /// A block consists of one or several variables. They appear in the order they're present in this vector.
        std::vector<NetMessageVariable> variables;
        /// Specifies the multiplicity of this block. If repeatCount == NetMessageBlock::cVariableCount, then the number of blocks
        /// is given by a single byte in the network packet.
        ///\todo Is this needed? Doesn't "NetMessageBlockType == NetBlockVariable"  indicate this also?
        static const int cVariableCount = -1;
    };

    // Probably going to remove this, useless (for now at least).
    /*enum NetPriorityLevel
    {
        NetPriorityInvalid = 0,
        NetPriorityHigh,
        NetPriorityMedium,
        NetPriorityLow,
        NetPriorityEnumCount
    };*/

    /// Identifies different UDP message packets.
    /// \ingroup OpenSimProtocolClient
    typedef unsigned long NetMsgID;

    /// Describes the format of an UDP network message.
    /// \ingroup OpenSimProtocolClient
    struct NetMessageInfo
    {
        std::string name;
        std::vector<NetMessageBlock> blocks;

        NetMsgID id;
        NetTrustLevel trustLevel;
        NetEncoding encoding;
    };

}

#endif // incl_ProtocolUtilities_NetMessage_h

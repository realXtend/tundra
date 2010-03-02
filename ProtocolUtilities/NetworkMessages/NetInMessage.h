// For conditions of distribution and use, see copyright notice in license.txt
#ifndef incl_ProtocolUtilities_NetInMessage_h
#define incl_ProtocolUtilities_NetInMessage_h

#include "RexTypes.h"
#include "NetMessageList.h"
#include "NetMessageException.h"
#include "Quaternion.h"

using namespace RexTypes;

class RexUUID;

namespace ProtocolUtilities
{

    /** Helps parsing inbound packets by supporting convenient reading of new data from the message. Also
        tracks that the message is read with the right structure.
        \ingroup OpenSimProtocolClient */
        
    class NetInMessage
    {
    public:
        NetInMessage(size_t seqNum, const uint8_t *compressedData, size_t numBytes, bool zeroEncoded);
        ~NetInMessage();
        
        NetInMessage(const NetInMessage &rhs);
        
        // The following functions all read data from the message and advance to the next variable in the message block.
        uint8_t  ReadU8();
        uint16_t ReadU16();
        uint32_t ReadU32();
        uint64_t ReadU64();

        int8_t  ReadS8();
        int16_t ReadS16();
        int32_t ReadS32();
        int64_t ReadS64();

        float  ReadF32();
        double ReadF64();

        bool ReadBool();

        Vector3 ReadVector3();
        Vector3d ReadVector3d();
        Vector4 ReadVector4();
        Quaternion ReadQuaternion();

        RexUUID ReadUUID();

        //IPADDR    ReadIPAddr(IPADDR value);  ///\todo
        //IPPORT    ReadIPPort(IPPORT value);  ///\todo

        void ReadString(char *dst, size_t maxSize);
        std::string ReadString();

        /// Use to read a generic buffer of bytes from the stream. Use this to read a VarBufferXX variable.
        /// @param bytesRead [out] The number of bytes the returned buffer holds. Cannot pass in zero.
        /// @return A pointer to the memory area. The returned memory remains owned by NetInMessage so no need to free it.
        const uint8_t *ReadBuffer(size_t *bytesRead);
        
        /// Check the type of the next variable in the message.
        /// @return NetVariableType enum.
        NetVariableType CheckNextVariableType() const;

        /// @return The index of the current block we're reading, or >= this->GetBlockCount() if there are no more variables to read.
        size_t GetCurrentBlock() const { return currentBlock; }

        /// @return The index of the current variable we're reading. If GetCurrentBlock()==GetBlockCount(), then the return value is undefined.
        size_t GetCurrentVariable() const { return currentVariable; }

        /// @return The number of blocks in this message.
        size_t GetBlockCount() const { return messageInfo->blocks.size(); }

        /// @return The number of instances of the current block we're reading.
        size_t ReadCurrentBlockInstanceCount();

        /// @return The number of bytes the next variable to-be-read takes.
        size_t ReadVariableSize() const { return currentVariableSize; }

        /// Called to skip to the next variable. Same as private function AdvanceToNextVariable but additionally makes it possible not to increment bytesRead
        /// if data is already read (and bytesRead incremented) with ReadBytesUnchecked. Increments bytesRead the right amount also when a NetVarBufferxxx is skipped.
        void SkipToNextVariable(bool bytesAlreadyRead = false);
        
        /// Skips to the first variable with the specified name.
        void SkipToFirstVariableByName(const std::string &variableName);

        /// Skips all the remaining variables in the current instance of the current block.
        /// The new read pointer will point to the first variable of the next instance of current block type, or if this was the last instance of current block type,
        /// then to the first variable of the first instance of the next block type (if it exists).
        void SkipToNextInstanceStart();

        /// Returns a pointer to a stream of given amount of bytes in the inbound packet. Doesn't do any validation. Increments the bytesRead pointer, but doesn't
        /// advance to the next variable. Use this only to do custom raw reading of the packet.
        /// @param Amount of bytes to be read.
        void *ReadBytesUnchecked(size_t count);
        
        /// @return The message's sequence number.
        uint32_t GetSequenceNumber() const { return sequenceNumber; } 
        
        /// @return Message id of this packet.
        const NetMsgID GetMessageID() const { return messageID; }

        void SetMessageInfo(const NetMessageInfo *info);

        /// @return A structure that represents the types and amounts of blocks and variables of this packet. Use it to examine the whole structure of this message type.
        const NetMessageInfo *GetMessageInfo() const { return messageInfo; }

        /// @return The original message data.
        const std::vector<uint8_t> &GetData() const { return messageData; }

        /// @return The size of the data (message body, the header is excluded). 
        size_t GetDataSize() const { return messageData.size(); }

        /// @return The amount of read bytes.
        uint32_t BytesRead() const { return (uint32_t)bytesRead; }

        /// Resets the reading of the message stream and jumps back to the first block & variable.
        void ResetReading();

#ifdef _DEBUG
        /// Overrides the MsgID of this message with another. Only for debugging purposes.
        void SetMessageID(NetMsgID id);
#endif
    private:
        void operator=(const NetInMessage &);
        
        /// Called to start reading the next variable.
        void AdvanceToNextVariable();

        /// @return Reads the block count for the next occurring block, a block type "Variable" or "Multiple".
        void StartReadingNextBlock();

        /// Reads the size the next variable in the message.
        void ReadNextVariableSize();

        /// Jumps all internal pointers to denote there is no more packet data to read.
        void SkipToPacketEnd();
        
        /// Checks the unrestrictedness of the reading operation.
        void RequireNextVariableType(NetVariableType type);
        
        /// The sequence number of the message.
        uint32_t sequenceNumber;

        /// The message ID of the message (extracted from the message stream).
        uint32_t messageID;

        /// Identifies what kind of packet we're handling.
        const NetMessageInfo *messageInfo;
        
        /// A pointer to the inbound message body
        std::vector<uint8_t> messageData;
        
        /// Index of the current block.
        size_t currentBlock;
        
        /// Index that denotes which of the instances of the current block we're reading (if there are multiple)
        size_t currentBlockInstanceNumber;
        
        /// Tells how many times the current block is present in the stream.
        size_t currentBlockInstanceCount;
        
        /// Index of the current variable.
        size_t currentVariable;
        
        /// The size of the current variable, in bytes.
        size_t currentVariableSize;
        
        /// How many bytes of the packet we have read.
        size_t bytesRead;
        
        /// If true, the next block to-be-come has variable count of instances.
        bool variableCountBlockNext;
    };

}

#endif // incl_ProtocolUtilities_NetInMessage_h

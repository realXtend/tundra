// For conditions of distribution and use, see copyright notice in license.txt
#ifndef incl_Rex_NetOutMessage_h
#define incl_Rex_NetOutMessage_h

#include <sys/types.h>
#include <ctime>

#include "RexTypes.h"
#include "NetMessageList.h"
#include "Quaternion.h"

using namespace RexTypes;
class NetMessageManager;
class RexUUID;

namespace ProtocolUtilities
{

    /** Helps building outbound packets by supporting convenient addition of new data to the message. Also
        tracks that the message is crafted with the right structure.
        \ingroup OpenSimProtocolClient */
    class NetOutMessage
    {
    public:
        NetOutMessage();
        ~NetOutMessage();

        NetOutMessage(const NetOutMessage &rhs)
        {
            messageInfo = rhs.messageInfo;
            messageData = rhs.messageData;
            bytesFilled = rhs.bytesFilled;
            sequenceNumber = rhs.sequenceNumber;
            currentBlock = rhs.currentBlock;
            currentVariable = rhs.currentVariable;
            blockQuantityCounter = rhs.blockQuantityCounter;        
        }
        
        // The following functions all append data into the message. The way this works is that the application calls the following AddX functions in the order
        // the protocol specifies the variables to be sent. Internal tracking mechanisms are used to remember which blocks/variables have been filled so far.
        void AddU8(uint8_t value);
        void AddU16(uint16_t value);
        void AddU32(uint32_t value);
        void AddU64(uint64_t value);
        void AddS8(int8_t value);
        void AddS16(int16_t value);
        void AddS32(int32_t value);
        void AddS64(int64_t value);
        void AddF32(float value);
        void AddF64(double value);
        void AddVector3(const Vector3 &value);
        void AddVector3d(const Vector3d &value);
        void AddVector4(const Vector4 &value);
        void AddQuaternion(const Quaternion &value);
        void AddUUID(const RexUUID &value);
        void AddBool(bool value);
        //void AddIPAddr(IPADDR value); ///\todo
        //void AddIPPort(IPPORT value); ///\todo
        //void AddNetVarFixed ///\todo Is this needed?

        /// Use to append a generic buffer of bytes into the stream. Use this to fill a VarBufferXX variable.
        void AddBuffer(size_t count, uint8_t *data);

        /// Use to append a string (including the end zero) as a generic buffer of bytes into the stream.
        void AddString(const char* str);

        /// Use to append a string (including the end zero) as a generic buffer of bytes into the stream.
        void AddString(const std::string& str);
        
        /// Check the type of the next variable that is expected to be present in the message.
        NetVariableType CheckNextVariable() const;
        
        /// Sets variable block count for block type "Variable".
        void SetVariableBlockCount(size_t var_count);
            
        /// Appends a stream of bytes into the outbound packet. Doesn't do any validation. Use this only to craft custom raw message packets outside the protocol.
        void AddBytesUnchecked(size_t count, const void *data);
            
        /// Moves the internal byte pointer to the start of the next variable. Call this only to craft custom raw message packets outside the protocol.
        void AdvanceToNextVariable();

        /// Marks the message as reliable
        void MarkReliable() { messageData[0] |= NetFlagReliable; }
        
        /// Marks the message as a resend
        void MarkResend() { messageData[0] |= NetFlagResent; }
        
        /// @return True if this message is flagged as reliable, i.e. if the receiver has to Ack it.
        bool IsReliable() const { return (messageData[0] & NetFlagReliable) != 0;}
        
        /// @return The ID that is associated to this packet type.
        NetMsgID GetMessageID() const { return messageInfo->id; }
        
        /// @return The number of filled bytes in the message so far, including message header.
        uint32_t BytesFilled() const { return (uint32_t)bytesFilled; }
        
        /// @return The type of the packet we're building.
        const NetMessageInfo *GetMessageInfo() const { return messageInfo; }

        /// @return The raw message buffer where the packet is constructed. Use this only to craft custom raw messages without validation.
        std::vector<uint8_t> &GetData() { return messageData; }

        /// @return The sequence number for the packet we're building. This method is not meaningful for end users, as the seqNum is created only when the message
        /// is sent out to the stream.
        uint32_t GetSequenceNumber() const { return sequenceNumber; }
        
        /// Resets and clears the message stream and jumps back to the first block & variable.
        void ResetWriting();

        /// Adds the header information to the message data.
        void AddMessageHeader();

        /// VLE-encodes the sequence number to the header (bytes 1-4).
        void SetSequenceNumber(size_t seqNum);
        
        /// Set the type of the packet we're building.
        void SetMessageInfo(const NetMessageInfo *info);

    private: // friend-public:
        void operator=(const NetOutMessage &);
        
        // NetMessageManager manages the internal header fields of the message, but this can't all be done ctor-time.
        friend class NetMessageManager;

    private: // friend-private:
        /// Contains the buffer of the serialized (incomplete) message.
        std::vector<uint8_t> messageData;
        
        /// Identifies what kind of packet we're building.
        const NetMessageInfo *messageInfo;
        
        /// How many bytes of the packet have we already built.
        size_t bytesFilled;
        
        /// The sequence number of the packet.
        uint32_t sequenceNumber;
        
        /// Index of the current block.
        size_t currentBlock;
        
        /// Index of the current variable.
        size_t currentVariable;
        
        /// Keeps count how many times the same block must be repeated.
        size_t blockQuantityCounter;
    };

}

#endif

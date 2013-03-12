#pragma once

#include "kNet/DataDeserializer.h"
#include "kNet/DataSerializer.h"
#include "Math/float3.h"

/// Network message for camera orientation.
struct MsgCameraOrientationRequest
{
        MsgCameraOrientationRequest()
        {
            InitToDefault();
        }

        MsgCameraOrientationRequest(const char *data, size_t numBytes)
        {
            InitToDefault();
            kNet::DataDeserializer dd(data, numBytes);
            DeserializeFrom(dd);
        }

        void InitToDefault()
        {
            reliable = defaultReliable;
            inOrder = defaultInOrder;
            priority = defaultPriority;
        }

        enum { messageID = 105 };
        static inline const char * const Name() { return "CameraOrientationRequest"; }

        static const bool defaultReliable = true;
        static const bool defaultInOrder = true;
        static const u32 defaultPriority = 100;

        bool reliable;
        bool inOrder;
        u32 priority;

        u8 enableCameraUpdates;

        inline size_t Size() const
        {
            return 1;
        }

        inline void SerializeTo(kNet::DataSerializer &dst) const
        {
            dst.Add<u8>(enableCameraUpdates);
        }

        inline void DeserializeFrom(kNet::DataDeserializer &src)
        {
            enableCameraUpdates = src.Read<u8>();
        }

};


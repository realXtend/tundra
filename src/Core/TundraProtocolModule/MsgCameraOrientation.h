#pragma once

#include "kNet/DataDeserializer.h"
#include "kNet/DataSerializer.h"
#include "Math/float3.h"

/// Network message for camera orientation.
struct MsgCameraOrientation
{
        MsgCameraOrientation()
        {
            InitToDefault();
        }

        MsgCameraOrientation(const char *data, size_t numBytes)
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

        enum { messageID = 104 };
        static inline const char * const Name() { return "CameraOrientation"; }

        static const bool defaultReliable = true;
        static const bool defaultInOrder = true;
        static const u32 defaultPriority = 100;

        bool reliable;
        bool inOrder;
        u32 priority;

        float orientationx;
        float orientationy;
        float orientationz;
        float orientationw;

        float positionx;
        float positiony;
        float positionz;


        inline size_t Size() const
        {
            return 4 + 4 + 4 + 4 + 4 + 4 + 4;
        }

        inline void SerializeTo(kNet::DataSerializer &dst) const
        {
            dst.Add<float>(orientationx);
            dst.Add<float>(orientationy);
            dst.Add<float>(orientationz);
            dst.Add<float>(orientationw);
            dst.Add<float>(positionx);
            dst.Add<float>(positiony);
            dst.Add<float>(positionz);
        }

        inline void DeserializeFrom(kNet::DataDeserializer &src)
        {
            orientationx = src.Read<float>();
            orientationy = src.Read<float>();
            orientationz = src.Read<float>();
            orientationw = src.Read<float>();
            positionx = src.Read<float>();
            positiony = src.Read<float>();
            positionz = src.Read<float>();
        }

};


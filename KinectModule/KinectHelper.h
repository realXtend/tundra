// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_KinectModule_KinectHelper_h
#define incl_KinectModule_KinectHelper_h

#include <ObjBase.h>        // This defines 'interface' as struct, needed with MSR_NuiApi.h
#include <MSR_NuiApi.h>     // Microsoft Kinect SDK API header

#include <QVariant>
#include <QVector4D>
#include <QPainter>
#include <QRectF>

class KinectHelper
{
    public:
        /// Constructor.
        KinectHelper();

        /// Deconstructor.
        virtual ~KinectHelper();

        /// Convert depth data to a rgb pixel data.
        /// @param USHORT pixel to convert.
        /// @return RGBQUAD converted pixel.
        RGBQUAD ConvertDepthShortToQuad(USHORT s);

        /// Convert Kinect SDK bone index to a string id.
        /// @param int Index of the bone.
        /// @return QString A string form id for the bone index.
        QString ConvertBoneIndexToId(int index);

        /// Convert Kinect SDK Vector4 to QVector4D.
        /// @param Vector4 Kinect SDK Vector4.
        /// @return QVector4D The Converted vector.
        QVector4D ConvertVector4ToQVector4D(::Vector4 kinectVector);

        /// Connect two bones with a line in the given painter.
        /// @param QPainter* Painter to draw the bone line.
        /// @param QRectF Rect of the drawing surface.
        /// @param QVariant Bone 1 position.
        /// @param QVariant Bone 2 position.
        void ConnectBones(QPainter *p, QRectF pRect, QVariant pos1, QVariant pos2);

        /// Parses Kinect SDK skeleton data struct into a QVariantMap.
        /// @param int Skeleton index.
        /// @param NUI_SKELETON_DATA Source data struct.
        QVariantMap ParseSkeletonData(int skeletonIndex, NUI_SKELETON_DATA skeletonData);
};

#endif

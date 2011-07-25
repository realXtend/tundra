// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "KinectHelper.h"

#include <QDebug>

KinectHelper::KinectHelper()
{
}

KinectHelper::~KinectHelper()
{
}

QVariantMap KinectHelper::ParseSkeletonData(int skeletonIndex, NUI_SKELETON_DATA skeletonData)
{
    QVariantMap data;

    data["tracking-id"] = (unsigned int)skeletonData.dwTrackingID;
    data["enrollment-index"] = (unsigned int)skeletonData.dwEnrollmentIndex;
    data["user-index"] = (unsigned int)skeletonData.dwUserIndex;
    
    data["skeleton-index"] = (unsigned int)skeletonIndex;
    data["skeleton-position"] = ConvertVector4ToQVector4D(skeletonData.Position);

    for (int i=0; i < NUI_SKELETON_POSITION_COUNT; ++i)
        data[ConvertBoneIndexToId(i)] = ConvertVector4ToQVector4D(skeletonData.SkeletonPositions[i]);

    return data;
}

QVector4D KinectHelper::ConvertVector4ToQVector4D(::Vector4 kinectVector)
{
    QVector4D v;
    v.setX(kinectVector.x);
    v.setY(kinectVector.y);
    v.setZ(kinectVector.z);
    v.setW(kinectVector.w);
    return v;
}

QString KinectHelper::ConvertBoneIndexToId(int index)
{
    QString base = "bone-";
    switch (index)
    {
        case NUI_SKELETON_POSITION_HIP_CENTER:
            return base + "hip-center";
        case NUI_SKELETON_POSITION_SPINE:
            return base + "spine";
        case NUI_SKELETON_POSITION_SHOULDER_CENTER:
            return base + "shoulder-center";
        case NUI_SKELETON_POSITION_HEAD:
            return base + "head";
        case NUI_SKELETON_POSITION_SHOULDER_LEFT:
            return base + "shoulder-left";
        case NUI_SKELETON_POSITION_ELBOW_LEFT:
            return base + "elbow-left";
        case NUI_SKELETON_POSITION_WRIST_LEFT:
            return base + "wrist-left";
        case NUI_SKELETON_POSITION_HAND_LEFT:
            return base + "hand-left";
        case NUI_SKELETON_POSITION_SHOULDER_RIGHT:
            return base + "shoulder-right";
        case NUI_SKELETON_POSITION_ELBOW_RIGHT:
            return base + "elbow-right";
        case NUI_SKELETON_POSITION_WRIST_RIGHT:
            return base + "wrist-right";
        case NUI_SKELETON_POSITION_HAND_RIGHT:
            return base + "hand-right";
        case NUI_SKELETON_POSITION_HIP_LEFT:
            return base + "hip-left";
        case NUI_SKELETON_POSITION_KNEE_LEFT:
            return base + "knee-left";
        case NUI_SKELETON_POSITION_ANKLE_LEFT:
            return base + "ankle-left";
        case NUI_SKELETON_POSITION_FOOT_LEFT:
            return base + "foot-left";
        case NUI_SKELETON_POSITION_HIP_RIGHT:
            return base + "hip-right";
        case NUI_SKELETON_POSITION_KNEE_RIGHT:
            return base + "knee-right";
        case NUI_SKELETON_POSITION_ANKLE_RIGHT:
            return base + "ankle-right";
        case NUI_SKELETON_POSITION_FOOT_RIGHT:
            return base + "foot-right";
        default:
            return base + "unknown-" + QString::number(index);
    }
}

RGBQUAD KinectHelper::ConvertDepthShortToQuad(USHORT s)
{
    USHORT RealDepth = (s & 0xfff8) >> 3;
    USHORT Player = s & 7;

    // transform 13-bit depth information into an 8-bit intensity appropriate
    // for display (we disregard information in most significant bit)
    BYTE l = 255 - (BYTE)(256*RealDepth/0x0fff);

    RGBQUAD q;
    q.rgbRed = q.rgbBlue = q.rgbGreen = 0;

    switch( Player )
    {
        case 0:
        {
            q.rgbRed = l / 2;
            q.rgbBlue = l / 2;
            q.rgbGreen = l / 2;
            break;
        }
        case 1:
        {
            q.rgbRed = l;
            break;
        }
        case 2:
        {
            q.rgbGreen = l;
            break;
        }
        case 3:
        {
            q.rgbRed = l / 4;
            q.rgbGreen = l;
            q.rgbBlue = l;
            break;
        }
        case 4:
        {
            q.rgbRed = l;
            q.rgbGreen = l;
            q.rgbBlue = l / 4;
            break;
        }
        case 5:
        {
            q.rgbRed = l;
            q.rgbGreen = l / 4;
            q.rgbBlue = l;
            break;
        }
        case 6:
        {
            q.rgbRed = l / 2;
            q.rgbGreen = l / 2;
            q.rgbBlue = l;
            break;
        }
        case 7:
        {    
            q.rgbRed = 255 - ( l / 2 );
            q.rgbGreen = 255 - ( l / 2 );
            q.rgbBlue = 255 - ( l / 2 );
            break;
        }
    }

    return q;
}

void KinectHelper::ConnectBones(QPainter *p, QRectF pRect, QVariant vec1, QVariant vec2)
{
    if (!vec1.canConvert<QVector4D>() || !vec2.canConvert<QVector4D>())
        return;

    QVector4D qvec1 = qvariant_cast<QVector4D>(vec1);
    qvec1 *= 200;
    QPointF pos1 = pRect.center() + qvec1.toPointF();

    QVector4D qvec2 = qvariant_cast<QVector4D>(vec2);
    qvec2 *= 200;
    QPointF pos2 = pRect.center() + qvec2.toPointF();

    p->drawLine(pos1, pos2);
}

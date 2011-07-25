// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "KinectDevice.h"

KinectDevice::KinectDevice(QObject *parent) :
    QObject(parent),
    isTrackingSkeletons_(false)
{
}

KinectDevice::~KinectDevice()
{
}

void KinectDevice::DoVideoUpdate(const QImage &image)
{
    emit VideoUpdate(image);
}

void KinectDevice::DoDepthUpdate(const QImage &image)
{
    emit DepthUpdate(image);
}

void KinectDevice::DoSkeletonUpdate(const QVariantMap &data)
{
    if (!isTrackingSkeletons_)
        SetSkeletonTracking(true);
    emit SkeletonUpdate(data);
}

void KinectDevice::SetSkeletonTracking(bool tracking)
{
    isTrackingSkeletons_ = tracking;
    emit TrackingSkeleton(tracking);
}

bool KinectDevice::IsTrackingSkeletons()
{
    return isTrackingSkeletons_;
}

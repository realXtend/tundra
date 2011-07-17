// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "KinectDevice.h"

KinectDevice::KinectDevice(const QString &name) :
    IDevice(name),
    isTrackingSkeletons_(false)
{
}

KinectDevice::~KinectDevice()
{
}

bool KinectDevice::InternalInitialize()
{
    /// For now nothing to initialize
    return true;
}

bool KinectDevice::InternalStart()
{
    /// For now there is not start or stop for the kinect device
    return true;
}

bool KinectDevice::InternalStop()
{
    /// For now there is not start or stop for the kinect device
    return true;
}

void KinectDevice::InternalUpdate(float frametime)
{
    /// Nothing to poll for Kinect, there is a threading monitoring kinect events in KinectModule
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

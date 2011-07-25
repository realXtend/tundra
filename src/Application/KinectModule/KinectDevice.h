// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_KinectModule_KinectDevice_h
#define incl_KinectModule_KinectDevice_h

#include "IDevice.h"
#include "KinectAPI.h"

#include <QObject>
#include <QString>
#include <QImage>
#include <QVariantMap>

/*!
    KinectDevice is the way to get data from a Microsoft Kinect device.
    The KinectModule reads all available data from the device and emits the Tundra/Qt style data with this device.

    You can acquire the Kinect device via DevicesAPI with name "kinect" and connecting to the signals specified in this class:
    VideoUpdate, DepthUpdate, SkeletonUpdate and TrackingSkeleton.

    \code
    IDevice *kinect = framework_->Devices()->GetDevice("kinect");
    if (kinect)
        connect(kinect, SIGNAL(SkeletonUpdate(const QVariantMap)), SLOT(HandleSkeletonData(const QVariantMap)));
    \endcode
 */
class KINECT_VOIP_MODULE_API KinectDevice : public IDevice
{

Q_OBJECT

friend class KinectModule;

public:
    KinectDevice(const QString &name);
    ~KinectDevice();

public slots:
    /// Get current skeleton tracking state.
    /// @return bool True if currently tracking and emitting SkeletonUpdate signal, false if we lost the skeleton(s).
    bool IsTrackingSkeletons();

signals:
    /// Emitted when a new video image is received from Kinect.
    /// @param QImage Most recent video frame.
    void VideoUpdate(const QImage image);
    
    /// Emitted when a new depth image is received from Kinect.
    /// @param QImage Most recent depth frame.
    void DepthUpdate(const QImage image);

    /// Emitted when a new skeleton data is received from Kinect.
    /// @param QVariantMap QString to QVariant map of skeleton tracking data. Including all bone position and skeleton index (tracking max 2 skeletons).
    /// @todo Document the id to value pairs properly so people can use this data without printing the map and making assumptions!
    void SkeletonUpdate(const QVariantMap data);

    /// Emitted when skeleton tracking is started or stopped. This can be handy for eg. reseting things when we lose skeleton tracking.
    /// @param bool True if currently tracking and emitting SkeletonUpdate signal, false if we lost the skeleton(s).
    void TrackingSkeleton(bool tracking);

protected:
    /// This function gets called by KinectModule when its outside the Kinect processing thread, do not call this from 3rd party code!
    void DoVideoUpdate(const QImage &image);

    /// This function gets called by KinectModule when its outside the Kinect processing thread, do not call this from 3rd party code!
    void DoDepthUpdate(const QImage &image);

    /// This function gets called by KinectModule when its outside the Kinect processing thread, do not call this from 3rd party code!
    void DoSkeletonUpdate(const QVariantMap &data);

    /// This function gets called by KinectModule when its outside the Kinect processing thread, do not call this from 3rd party code!
    void SetSkeletonTracking(bool tracking);

    /// IDevice override.
    virtual bool InternalInitialize();

    /// IDevice override.
    virtual bool InternalStart();

    /// IDevice override.
    virtual bool InternalStop();

    /// IDevice override.
    void InternalUpdate(float frametime);

private:
    bool isTrackingSkeletons_;

};

#endif

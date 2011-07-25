// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include "KinectAPI.h"

#include <QObject>
#include <QString>
#include <QImage>
#include <QVariantMap>

/*! KinectDevice is the way to get data from a Microsoft Kinect device.
    The KinectModule reads all available data from the device and emits the Tundra/Qt style data with this device.

    You can acquire the Kinect device via Frameworks dynamic object with name "kinect" and connecting to the signals specified in this class:
    VideoUpdate, DepthUpdate, SkeletonUpdate and TrackingSkeleton.

    \code
    C++:
        // In C++ framework properties are a bit inconvenient, but available non the less.
        QVariant kinectVariant = GetFramework()->property("kinect");
        if (kinectVariant.isNull())
        {
            LogWarning("Kinect module not present, oops!");
            return;
        }
        if (kinectVariant.canConvert<QObject*>())
        {
            QObject *kinectPtr = kinectVariant.value<QObject*>();
            if (kinectPtr)
            {
                QObject::connect(kinectPtr, SIGNAL(VideoUpdate(const QImage)), this, SLOT(OnKinectVideo(const QImage)));
                QObject::connect(kinectPtr, SIGNAL(SkeletonUpdate(const QVariantMap)), this, SLOT(OnKinectSkeleton(const QVariantMap)));
                QObject::connect(kinectPtr, SIGNAL(TrackingSkeleton(bool)), this, SLOT(OnKinectTrackingSkeleton(bool)));
            }
        }

    JavaScript:
        // All framework dynamic objects are automatically visible in JavaScript.
        if (kinect == null) {
            print("Kinect module not present, oops!");
            return;
        }
        var isTrackingRightNow = kinect.IsTrackingSkeletons();
        kinect.VideoUpdate.connect(onKinectVideo);
        kinect.SkeletonUpdate.connect(onKinectSkeleton);
        kinect.TrackingSkeleton.connect(onKinectTrackingSkeleton);

    Python:
        The KinectDevice class needs to be registered to PythonQt so the QObject is knows and can be used.
        This is not done yet so python access is not working yet.
    \endcode
 */
class KINECT_VOIP_MODULE_API KinectDevice : public QObject
{

Q_OBJECT

friend class KinectModule;

public:
    KinectDevice(QObject *parent);
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

private:
    bool isTrackingSkeletons_;

};

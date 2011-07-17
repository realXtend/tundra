// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_KinectModule_h
#define incl_KinectModule_h

#include "IModule.h"
#include "KinectAPI.h"
#include "KinectFwd.h"

#include <windows.h>        // Generally needed for this modules code, windows HRESULT, HANDLE etc.
#include <ObjBase.h>        // This defines 'interface' as struct, needed with MSR_NuiApi.h
#include <MSR_NuiApi.h>     // Microsoft Kinect SDK API header

#include "KinectHelper.h"   // Include after the above

#include <QObject>
#include <QVariant>
#include <QList>
#include <QLabel>
#include <QSize>

/*!
    KinectModule reads Kinect with Microsoft Kinect SDK. A device with name "kinect" is
    created into DevicesAPI that 3rd party can utilize to get the kinect data.
 
    See KinectDevice for the specific signals and instructions.
 */
class KINECT_VOIP_MODULE_API KinectModule : public QObject, public IModule
{

Q_OBJECT

public:
    /// Constructor
    KinectModule();

    /// Deconstructor
    virtual ~KinectModule();

    /// IModule override
    void PostInitialize();

    /// IModule override
    void Uninitialize();

private slots:
    /// Callback from Kinect processing thread.
    /// @note Do not call this from outside KinectModule.
    void OnVideoReady(const QImage videoImage);

    /// Callback from Kinect processing thread.
    /// @note Do not call this from outside KinectModule.
    void OnDepthReady(const QImage depthImage);

    /// Callback from Kinect processing thread.
    /// @note Do not call this from outside KinectModule.
    void OnSkeletonReady(const QVariantMap skeletonData);

    /// Callback from Kinect processing thread.
    /// @note Do not call this from outside KinectModule.
    void OnSkeletonTracking(bool tracking);

signals:
    /// Signal that is emitted from the Kinect processing thread.
    /// @note Do not connect to this signal, use KinectDevice instead.
    void VideoReady(const QImage videoImage);

    /// Signal that is emitted from the Kinect processing thread.
    /// @note Do not connect to this signal, use KinectDevice instead.
    void DepthReady(const QImage depthImage);

    /// Signal that is emitted from the Kinect processing thread.
    /// @note Do not connect to this signal, use KinectDevice instead.
    void SkeletonReady(const QVariantMap skeletonData);

    /// Signal that is emitted from the Kinect processing thread.
    /// @note Do not connect to this signal, use KinectDevice instead.
    void SkeletonTracking(bool tracking);

private:
    /// Kinect processing thread that listens to the Kinect SDK events.
    static DWORD WINAPI KinectProcessThread(LPVOID pParam);

    /// Callback for when video is available.
    /// @note We are inside the Kinect processing thread when this function is called. Do not call this outside KinectModule.
    void GetVideo();
    
    /// Callback for when depth is available.
    /// @note We are inside the Kinect processing thread when this function is called. Do not call this outside KinectModule.
    void GetDepth();
    
    /// Callback for when skeleton is available.
    /// @note We are inside the Kinect processing thread when this function is called. Do not call this outside KinectModule.
    void GetSkeleton();

    /// Debug draw skeleton data (if available) into a QWidget.
    /// @note This function does nothing if debugging_ is false.
    void DrawDebugSkeletons();

    static std::string module_name_;

    HANDLE kinectProcess_;
    HANDLE kinectDepthStream_;
    HANDLE kinectVideoStream_;

    HANDLE eventKinectProcessStop_;
    HANDLE eventDepthFrame_;
    HANDLE eventVideoFrame_;
    HANDLE eventSkeletonFrame_;

    KinectHelper helper_;
    KinectDevice *tundraKinectDevice_;

    QLabel *videoPreviewLabel_;
    QLabel *depthPreviewLabel_;
    QLabel *skeletonPreviewLabel_;

    QSize videoSize_;
    QSize depthSize_;

    bool kinectApiInitialized_;

    /// Set this to true if you want to see debug widgets and prints.
    bool debugging_;

    QList<QVariantMap > debugSkeletons_;
};


#endif

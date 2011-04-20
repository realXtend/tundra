// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_CameraInputModule_h
#define incl_CameraInputModule_h

#include "CameraInputModuleApi.h"
#include "Foundation.h"
#include "IModule.h"
#include "ModuleLoggingFunctions.h"

#include <QObject>
#include <QImage>

#include "opencv2/core/types_c.h"

class CameraInput;
class CvCapture;

/*! CameraInputModule reads a video camera (webcam) source from your PC and emits the image data as QImage. See more about the API from CameraInput.h.

    C++ via CameraInputModule:
    \code
    CameraInputModule *camModule = fw->GetModule<CameraInputModule>();
    if (camModule)
        connect(camModule->GetCameraInput(), SIGNAL(frameUpdate(const QImage&)), SLOT(Handler(const QImage&)));
    \endcode

    JavaScript via CameraInput object:
    \code
    // Note that frameUpdate will emit you a QImage. If you want to use it directly in eg. a QLabel
    // you can make it a QPixmap with QPixmap.fromImage(frame)
    camerainput.FrameUpdate.connect(Handler);
    \endcode

    This module depends and uses the opencv library http://opencv.willowgarage.com/wiki/
*/
class CAMERAINPUT_MODULE_API CameraInputModule : public QObject, public IModule
{

Q_OBJECT

public:
    /// Default constructor.
    CameraInputModule();

    /// Destructor.
    ~CameraInputModule();

    /// IModule override.
    void Initialize();

    /// IModule override.
    void Uninitialize();

    /// IModule override.
    void Update(f64 frametime);

public slots:
    /// Get CameraInput that has all the relevant slots and signals for camera data interaction.
    /// \return CameraInput* The CameraInput object ptr.
    CameraInput *GetCameraInput() const;

private slots:
    void GrabDevice();
    void ReleaseDevice();
    void OnCapturingStateChanged(bool capturing);
    bool UpdateSurfaces(const CvSize &size);

private:
    CameraInput *cameraInput_;
    CvCapture *openCvCamera_;

    IplImage *tchannel0;
    IplImage *tchannel1;
    IplImage *tchannel2;
    IplImage *tchannel3;
    IplImage *destFrame;

    qreal updateBuildup_;
    qreal updateInterval_;
};

#endif

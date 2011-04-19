// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_CameraInputModule_h
#define incl_CameraInputModule_h

#include "Foundation.h"
#include "IModule.h"
#include "ModuleLoggingFunctions.h"

#include <QObject>
#include <QImage>

#include "opencv2/core/types_c.h"

class CameraInput;
class CvCapture;

/*! CameraInputModule reads a video camera (webcam) source from your PC and emits the image data as QImage. See more about the API from CameraInput.h

    C++ via CameraInputModule:
    \code
    QObject *camModule = fw->GetModuleQObj("CameraInputModule");
    if (camModule)
        connect(camModule, SIGNAL(frameUpdate(const QImage&)), SLOT(Handler(const QImage&)));
    \endcode

    JavaScript via CameraInput object:
    \code
    camerainput.frameUpdate.connect(Handler);
    \endcode

    This module depends and uses the opencv library http://opencv.willowgarage.com/wiki/
*/
class CameraInputModule : public QObject, public IModule
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

signals:
    /// Replicates the CameraInput signal for easy c++ access to the camera data.
    void frameUpdate(const QImage &frame);

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

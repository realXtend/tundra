// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"

#include "CameraInputModule.h"
#include "CameraInput.h"

#include "LoggingFunctions.h"
DEFINE_POCO_LOGGING_FUNCTIONS("CameraInputModule")

#include "highgui.h"

#include <QLabel>
#include <QImage>

CameraInputModule::CameraInputModule() :
    IModule("CameraInputModule"),
    updateBuildup_(0.0),
    captureFps_(15),
    openCvCamera_(0),
    tchannel0(0),
    tchannel1(0),
    tchannel2(0),
    tchannel3(0),
    destFrame(0)
{
    SetCaptureFps(captureFps_);
}

CameraInputModule::~CameraInputModule()
{
}

void CameraInputModule::Initialize()
{
    cameraInput_ = new CameraInput(this, framework_);
    if (cameraInput_)
    {
        framework_->RegisterDynamicObject("camerainput", cameraInput_);
        connect(cameraInput_, SIGNAL(Capturing(bool)), SLOT(OnCapturingStateChanged(bool)));
    }
}

void CameraInputModule::Uninitialize()
{
    ReleaseDevice();
}

void CameraInputModule::SetCaptureFps(int fps)
{
    if (fps < 1)
        fps = 1;
    if (fps > 60)
        fps = 60;

    captureFps_ = fps;
    updateInterval_ = 1.0 / (qreal)fps;
}

CameraInput *CameraInputModule::GetCameraInput() const
{
    return cameraInput_;
}

void CameraInputModule::GrabDevice()
{
    if (framework_->IsHeadless())
        return;

    openCvCamera_ = cvCreateCameraCapture(0);
    if (openCvCamera_)
    {
        if (cameraInput_)
            cameraInput_->SetEnabled(true);
    }
    else
    {
        LogInfo("Could not find a camera device.");
        if (cameraInput_)
            cameraInput_->SetEnabled(false);
    }
}

void CameraInputModule::ReleaseDevice()
{
    if (openCvCamera_)
        cvReleaseCapture(&openCvCamera_);

    if (tchannel0)
        cvReleaseImage(&tchannel0);
    if (tchannel1)
        cvReleaseImage(&tchannel1);
    if (tchannel2)
        cvReleaseImage(&tchannel2);
    if (tchannel3)
        cvReleaseImage(&tchannel3);
    if (destFrame)
        cvReleaseImage(&destFrame);

    openCvCamera_ = 0;
    tchannel0 = 0;
    tchannel1 = 0;
    tchannel2 = 0;
    tchannel3 = 0;
    destFrame = 0;
}

void CameraInputModule::OnCapturingStateChanged(bool capturing)
{
    ReleaseDevice();
    if (capturing)
        GrabDevice();
}

void CameraInputModule::Update(f64 frametime)
{
    if (framework_->IsHeadless())
        return;
    if (!openCvCamera_)
        return;
    if (!cameraInput_)
        return;
    if (!cameraInput_->IsCapturing())
         return;

    updateBuildup_ += frametime;
    if (updateBuildup_ >= updateInterval_)
    {
        updateBuildup_ = 0;
        
        IplImage *currentFrame = cvQueryFrame(openCvCamera_);
        if (!currentFrame)
            return;

        // Read current frames size
        CvSize size = cvSize(currentFrame->width, currentFrame->height);

        // Check that surfaces are up to date
        if (!UpdateSurfaces(size))
            return;

        // Reset surface pixels to be sure
        cvSet(tchannel0, cvScalarAll(255), 0);
        cvSet(tchannel1, cvScalarAll(0), 0);
        cvSet(tchannel2, cvScalarAll(0), 0);
        cvSet(tchannel3, cvScalarAll(0), 0);

        // With currentFrame being the captured frame (3 channel RGB)
        // and destFrame the frame to be displayed
        cvSplit(currentFrame, tchannel1, tchannel2, tchannel3, NULL);
        cvMerge(tchannel1, tchannel2, tchannel3, tchannel0, destFrame);

        // Point to the image data stored in the IplImage*
        const unsigned char *data = (unsigned char *)(destFrame->imageData);

        // Create the QImage and feed it to CameraInput, it will them emit its signals accordingly.
        QImage image(data, size.width, size.height, QImage::Format_RGB32);
        if (!image.isNull() && image.width() > 0 && image.height() > 0)
            cameraInput_->SetFrame(image);
    }
}

bool CameraInputModule::UpdateSurfaces(const CvSize &size)
{
    // Prepare our image channels, if prepared check that they the are correct size.
    try 
    {
        if (!tchannel0)
        {
            tchannel0 = cvCreateImage(size, IPL_DEPTH_8U, 1);
        }
        else if (tchannel0->width != size.width || tchannel0->height != size.height)
        {
            cvReleaseImage(&tchannel0);
            tchannel0 = 0;
            tchannel0 = cvCreateImage(size, IPL_DEPTH_8U, 1);
        }

        if (!tchannel1)
        {
            tchannel1 = cvCreateImage(size, IPL_DEPTH_8U, 1);
        }
        else if (tchannel1->width != size.width || tchannel1->height != size.height)
        {
            cvReleaseImage(&tchannel1);
            tchannel1 = 0;
            tchannel1 = cvCreateImage(size, IPL_DEPTH_8U, 1);
        }

        if (!tchannel2)
        {
            tchannel2 = cvCreateImage(size, IPL_DEPTH_8U, 1);
        }
        else if (tchannel2->width != size.width || tchannel2->height != size.height)
        {
            cvReleaseImage(&tchannel2);
            tchannel2 = 0;
            tchannel2 = cvCreateImage(size, IPL_DEPTH_8U, 1);
        }

        if (!tchannel3)
        {
            tchannel3 = cvCreateImage(size, IPL_DEPTH_8U, 1);
        }
        else if (tchannel3->width != size.width || tchannel3->height != size.height)
        {
            cvReleaseImage(&tchannel3);
            tchannel3 = 0;
            tchannel3 = cvCreateImage(size, IPL_DEPTH_8U, 1);
        }

        if (!destFrame)
        {
            destFrame = cvCreateImage(size, IPL_DEPTH_8U, 4);
        }
        else if (destFrame->width != size.width || destFrame->height != size.height)
        {
            cvReleaseImage(&destFrame);
            destFrame = 0;
            destFrame = cvCreateImage(size, IPL_DEPTH_8U, 4);
        }
    }
    catch (...)
    {
        return false;
    }

    if (!tchannel0 || !tchannel1 || !tchannel2 || !tchannel3 || !destFrame)
        return false;
    return true;
}

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

POCO_BEGIN_MANIFEST(IModule)
   POCO_EXPORT_CLASS(CameraInputModule)
POCO_END_MANIFEST

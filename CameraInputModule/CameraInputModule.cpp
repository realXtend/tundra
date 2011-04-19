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
    openCvCamera_(0),
    updateBuildup_(0.0),
    updateInterval_(25.0 / 1000.0), // 25 fps
    tchannel0(0),
    tchannel1(0),
    tchannel2(0),
    tchannel3(0),
    destFrame(0)
{
}

CameraInputModule::~CameraInputModule()
{
}

void CameraInputModule::Initialize()
{
    cameraInput_ = new CameraInput(this, framework_);
    if (cameraInput_)
        framework_->RegisterDynamicObject("camerainput", cameraInput_);

    // Do not start reading camera input in headless mode. For example
    // on a headless server this would be bad as it would take up cpu for nothing.
    if (framework_->IsHeadless())
        return;

    openCvCamera_ = cvCreateCameraCapture(0);
    if (openCvCamera_)
    {
        LogInfo("Found a camera device.");
        if (cameraInput_)
            cameraInput_->SetEnabled(true);
    }
    else
        LogInfo("Could not find a camera device.");
}

void CameraInputModule::Uninitialize()
{
    if (openCvCamera_)
    {
        LogInfo("Releasing camera device and internal surfaces.");
        cvReleaseCapture(&openCvCamera_);
    }

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
}

void CameraInputModule::Update(f64 frametime)
{
    if (framework_->IsHeadless())
        return;
    if (!openCvCamera_)
        return;

    // Don't update too often, stick with 25 fps (updateInterval_)
    updateBuildup_ += frametime;
    if (updateBuildup_ >= updateInterval_)
    {
        updateBuildup_ = 0;
        
        IplImage *currentFrame = cvQueryFrame(openCvCamera_);
        if (!currentFrame)
            return;

        // read other parameters in local variables
        CvSize size = cvSize(currentFrame->width, currentFrame->height);

        // Check that surfaces are up to date
        if (!UpdateSurfaces(size))
            return;

        // Reset surface pixels to be sure
        cvSet(tchannel0, cvScalarAll(255), 0);
        cvSet(tchannel1, cvScalarAll(0), 0);
        cvSet(tchannel2, cvScalarAll(0), 0);
        cvSet(tchannel3, cvScalarAll(0), 0);

        // with cframe being the captured frame (3 channel RGB)
        // and dframe the frame to be displayed
        cvSplit(currentFrame, tchannel1, tchannel2, tchannel3, NULL);
        cvMerge(tchannel1, tchannel2, tchannel3, tchannel0, destFrame);

        // point to the image data stored in the IplImage*
        const unsigned char *data = (unsigned char *)(destFrame->imageData);

        // imageframe is my QLabel object
        QImage image(data, size.width, size.height, QImage::Format_RGB32);
        if (!image.isNull())
        {
            cameraInput_->SetFrame(image);
            emit frameUpdate(cameraInput_->CurrentFrame());
        }
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

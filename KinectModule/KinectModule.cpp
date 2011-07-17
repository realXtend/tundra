// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "KinectModule.h"
#include "KinectDevice.h"

#include "Framework.h"
#include "DevicesAPI.h"

#include <MMSystem.h>

#include <QImage>
#include <QVariant>
#include <QDebug>
#include <QPainter>
#include <QRectF>
#include <QPair>

#include "MemoryLeakCheck.h"

#include "LoggingFunctions.h"
DEFINE_POCO_LOGGING_FUNCTIONS("KinectModule")

std::string KinectModule::module_name_ = "KinectModule";

KinectModule::KinectModule() : 
    IModule(module_name_),
    kinectApiInitialized_(false),
    videoSize_(640, 480),
    depthSize_(320, 240),
    tundraKinectDevice_(0),
    eventDepthFrame_(0),
    eventVideoFrame_(0),
    eventSkeletonFrame_(0),
    eventKinectProcessStop_(0),
    kinectProcess_(0),
    videoPreviewLabel_(0),
    depthPreviewLabel_(0),
    skeletonPreviewLabel_(0),
    debugging_(false) // Change to true to see debug widgets and prints
{
    if (debugging_)
    {
        videoPreviewLabel_ = new QLabel();
        videoPreviewLabel_->setWindowTitle("Kinect Video Preview");
        depthPreviewLabel_ = new QLabel();
        depthPreviewLabel_->setWindowTitle("Kinect Depth Preview");
        skeletonPreviewLabel_ = new QLabel();
        skeletonPreviewLabel_->setWindowTitle("Kinect Skeleton Preview");
    }

    // These signals are emitted from the processing thread, 
    // so we need Qt::QueuedConnection to get it to the main thread
    connect(this, SIGNAL(VideoReady(const QImage)), SLOT(OnVideoReady(const QImage)), Qt::QueuedConnection);
    connect(this, SIGNAL(DepthReady(const QImage)), SLOT(OnDepthReady(const QImage)), Qt::QueuedConnection);
    connect(this, SIGNAL(SkeletonReady(const QVariantMap)), SLOT(OnSkeletonReady(const QVariantMap)), Qt::QueuedConnection);
    connect(this, SIGNAL(SkeletonTracking(bool)), SLOT(OnSkeletonTracking(bool)), Qt::QueuedConnection);
}

KinectModule::~KinectModule()
{
    SAFE_DELETE(videoPreviewLabel_);
    SAFE_DELETE(depthPreviewLabel_);
    SAFE_DELETE(skeletonPreviewLabel_);
}

void KinectModule::PostInitialize()
{
    QString message;
    HRESULT result = NuiInitialize(NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX | 
                                   NUI_INITIALIZE_FLAG_USES_SKELETON | 
                                   NUI_INITIALIZE_FLAG_USES_COLOR);
    if (SUCCEEDED(result))
    {
        LogInfo("Successfully found a Kinect device.");
        kinectApiInitialized_ = true;

        // Resolve the wanted image resolution
        NUI_IMAGE_RESOLUTION depthResolution = NUI_IMAGE_RESOLUTION_320x240;
        depthSize_ = QSize(320, 240);

        NUI_IMAGE_RESOLUTION videoResolution = NUI_IMAGE_RESOLUTION_INVALID;
        if (videoSize_.width() == 320 && videoSize_.height() == 240)
            videoResolution = NUI_IMAGE_RESOLUTION_320x240;
        else if (videoSize_.width() == 640 && videoSize_.height() == 480)
            videoResolution = NUI_IMAGE_RESOLUTION_640x480;
        if (videoResolution == NUI_IMAGE_RESOLUTION_INVALID)
        {
            videoResolution = NUI_IMAGE_RESOLUTION_640x480;
            videoSize_ = QSize(640, 480);
        }
        
        // Events
        eventDepthFrame_ = CreateEventA(NULL, TRUE, FALSE, NULL);
        eventVideoFrame_ = CreateEventA(NULL, TRUE, FALSE, NULL);
        eventSkeletonFrame_ = CreateEventA(NULL, TRUE, FALSE, NULL);

        // Skeleton
        result = NuiSkeletonTrackingEnable(eventSkeletonFrame_, 0);
        message = SUCCEEDED(result) ? "-- Skeleton tracking enabled" : "-- Skeleton tracking could not be enabled";
        LogInfo(message);
            
        // Image
        result = NuiImageStreamOpen(NUI_IMAGE_TYPE_COLOR, videoResolution,
                                    0, 2, eventVideoFrame_, &kinectVideoStream_);
        message = SUCCEEDED(result) ? "-- Image stream enabled" : "-- Image stream could not be enabled";
        LogInfo(message);

        // Depth
        result = NuiImageStreamOpen(NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX, depthResolution,
                                    0, 2, eventDepthFrame_, &kinectDepthStream_);
        message = SUCCEEDED(result) ? "-- Depth stream enabled" : "-- Depth stream could not be enabled";
        LogInfo(message);

        // Start the kinect processing thread
        eventKinectProcessStop_ = CreateEventA(NULL, FALSE, FALSE, NULL);
        kinectProcess_ = CreateThread(NULL, 0, KinectProcessThread, this, 0, NULL);

        // Register a Tundra Device
        tundraKinectDevice_ = new KinectDevice("kinect");
        if (GetFramework()->Devices()->RegisterDevice(tundraKinectDevice_))
            LogInfo("-- Registered 'kinect' device to DevicesAPI");
    }
    else
        LogWarning("Could not find a Kinect device.");
}

void KinectModule::Uninitialize()
{
    // Kill the Tundra kinect device
    SAFE_DELETE(tundraKinectDevice_);

    if (kinectApiInitialized_)
    {
        // Stop the Kinect processing thread
        if (eventKinectProcessStop_ != NULL)
        {
            // Signal the thread
            SetEvent(eventKinectProcessStop_);

            // Wait for thread to stop
            if (kinectProcess_ != NULL)
            {
                WaitForSingleObject(kinectProcess_, INFINITE);
                CloseHandle(kinectProcess_);
            }
            CloseHandle(eventKinectProcessStop_);
        }

        // Close related even handles
        if (eventSkeletonFrame_ && (eventSkeletonFrame_ != INVALID_HANDLE_VALUE))
        {
            CloseHandle(eventSkeletonFrame_);
            eventSkeletonFrame_ = 0;
        }
        if (eventDepthFrame_ && (eventDepthFrame_ != INVALID_HANDLE_VALUE))
        {
            CloseHandle(eventDepthFrame_);
            eventDepthFrame_ = 0;
        }
        if (eventVideoFrame_ && (eventVideoFrame_ != INVALID_HANDLE_VALUE))
        {
            CloseHandle(eventVideoFrame_);
            eventVideoFrame_ = 0;
        }

        // Shutdown Kinect lib
        NuiShutdown();
    }
}

DWORD WINAPI KinectModule::KinectProcessThread(LPVOID pParam)
{
    KinectModule *pthis = (KinectModule*)pParam;
    KinectHelper helper = pthis->helper_;

    HANDLE kinectEvents[4];
    int eventIndex;

    // Configure events to be listened on
    kinectEvents[0] = pthis->eventKinectProcessStop_;
    kinectEvents[1] = pthis->eventDepthFrame_;
    kinectEvents[2] = pthis->eventVideoFrame_;
    kinectEvents[3] = pthis->eventSkeletonFrame_;

    // Main thread loop
    while(1)
    {
        // Wait for an event to be signaled
        eventIndex = WaitForMultipleObjects(sizeof(kinectEvents) / sizeof(kinectEvents[0]), kinectEvents, FALSE, 100);
        if (eventIndex == 0)
            break;            

        // Process signal events
        switch (eventIndex)
        {
            case 1:
            {          
                pthis->GetDepth();
                break;
            }
            case 2:
            {
                pthis->GetVideo();
                break;
            }
            case 3:
            {
                pthis->GetSkeleton();
                break;
            }
            default:
                break;
        }
    }

    return 0;
}

void KinectModule::GetVideo()
{
    const NUI_IMAGE_FRAME *pImageFrame = NULL;
    KINECT_SURFACE_DESC textureDesc;

    HRESULT videoResult = NuiImageStreamGetNextFrame(kinectVideoStream_, 0, &pImageFrame);
    if (FAILED(videoResult))
        return;

    NuiImageBuffer *pTexture = pImageFrame->pFrameTexture;
    pTexture->GetLevelDesc(0, &textureDesc);

    KINECT_LOCKED_RECT LockedRect;
    HRESULT result = pTexture->LockRect(0, &LockedRect, NULL, 0);
    if (result == S_OK && LockedRect.Pitch != 0)
    {
        BYTE *videoBuffer = (BYTE*)LockedRect.pBits;

        QImage videoImage(videoBuffer, textureDesc.Width, textureDesc.Height, QImage::Format_RGB32);
        if (!videoImage.isNull())
            emit VideoReady(videoImage);

        pTexture->UnlockRect(0);
    }

    NuiImageStreamReleaseFrame(kinectVideoStream_, pImageFrame);
}

void KinectModule::OnVideoReady(const QImage videoImage)
{
    tundraKinectDevice_->DoVideoUpdate(videoImage);

    if (debugging_)
    {
        if (videoPreviewLabel_->size() != videoImage.size())
            videoPreviewLabel_->resize(videoImage.size());
        if (!videoPreviewLabel_->isVisible())
            videoPreviewLabel_->show();
        videoPreviewLabel_->setPixmap(QPixmap::fromImage(videoImage));
    }    
}

void KinectModule::GetDepth()
{
    const NUI_IMAGE_FRAME *depthImage = NULL;
    KINECT_LOCKED_RECT depthRect;

    HRESULT depthResult = NuiImageStreamGetNextFrame(kinectDepthStream_, 0, &depthImage);
    if (FAILED(depthResult))
        return;

    NuiImageBuffer *depthTexture = depthImage->pFrameTexture;
    depthResult = depthTexture->LockRect(0, &depthRect, NULL, 0);

    if (depthResult == S_OK && depthRect.Pitch != 0)
    {
        BYTE *depthByteBuffer = (BYTE*)depthRect.pBits;

        // Draw the bits to the bitmap
        RGBQUAD rgbrunTable[640 * 480]; // This will work even if our image is smaller
        RGBQUAD *rgbrun = rgbrunTable;
        USHORT *pBufferRun = (USHORT*)depthByteBuffer;
        for (int y=0; y<depthSize_.width(); y++)
        {
            for (int x=0; x<depthSize_.height(); x++)
            {
                RGBQUAD pixelRgb = helper_.ConvertDepthShortToQuad(*pBufferRun);
                *rgbrun = pixelRgb;
                
                pBufferRun++;
                rgbrun++;
            }
        }

        // Convert to a QImage
        QImage depthImage((BYTE*)rgbrunTable, depthSize_.width(), depthSize_.height(), QImage::Format_RGB32);
        if (!depthImage.isNull())
            emit DepthReady(depthImage);
    }
    else
        LogWarning("Buffer length of received texture is bogus!");

    NuiImageStreamReleaseFrame(kinectDepthStream_, depthImage);
}

void KinectModule::OnDepthReady(const QImage depthImage)
{
    tundraKinectDevice_->DoDepthUpdate(depthImage);
    
    if (debugging_)
    {
        if (depthPreviewLabel_->size() != depthImage.size())
            depthPreviewLabel_->resize(depthImage.size());
        if (!depthPreviewLabel_->isVisible())
            depthPreviewLabel_->show();
        depthPreviewLabel_->setPixmap(QPixmap::fromImage(depthImage));
    }
}

void KinectModule::GetSkeleton()
{
    NUI_SKELETON_FRAME skeletonFrame;
    HRESULT skeletonResult = NuiSkeletonGetNextFrame(0, &skeletonFrame);

    bool foundSkeletons = false;
    for (int i = 0; i < NUI_SKELETON_COUNT; i++)
        if (skeletonFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED)
            foundSkeletons = true;

    // Set our KinectDevices tracking state, this will be handy
    // on 3rd party using code to know when tracking data should be coming
    // to eg. reset things when skeleton tracking is lost
    debugSkeletons_.clear();
    if (!foundSkeletons)
    {
        if (tundraKinectDevice_->IsTrackingSkeletons())
            emit SkeletonTracking(false);
        return;
    }
    if (!tundraKinectDevice_->IsTrackingSkeletons())
        emit SkeletonTracking(true);

    // Smooth out the skeleton data, not sure what this does
    NuiTransformSmooth(&skeletonFrame, NULL);

    // Emit all of the found skeletons data events
    for (int i=0; i<NUI_SKELETON_COUNT; i++)
    {
        if (skeletonFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED)
        {
            QVariantMap tundraSkeletonData = helper_.ParseSkeletonData(i, skeletonFrame.SkeletonData[i]);
            
            emit SkeletonReady(tundraSkeletonData);

            if (debugging_)
                debugSkeletons_.append(tundraSkeletonData);
        }
    }
}

void KinectModule::OnSkeletonTracking(bool tracking)
{
    tundraKinectDevice_->SetSkeletonTracking(tracking);

    if (debugging_)
        LogInfo(QString("Tracking Skeletons    ") + (tracking ? "ON" : "OFF"));
}

void KinectModule::OnSkeletonReady(const QVariantMap skeletonData)
{
    tundraKinectDevice_->DoSkeletonUpdate(skeletonData);
    
    if (debugging_)
        DrawDebugSkeletons();
}

void KinectModule::DrawDebugSkeletons()
{
    if (!debugging_)
        return;
    if (debugSkeletons_.isEmpty())
        return;

    QRectF imgRect(0, 0, 640, 480);
    QSize imgSize = imgRect.size().toSize();
    QImage manipImage(imgSize, QImage::Format_RGB32);

    QPainter p(&manipImage);
    p.setPen(Qt::black);
    p.setBrush(QBrush(Qt::white));
    p.drawRect(imgRect);

    int skeletonIndex = 0;
    foreach(QVariantMap skeletonData, debugSkeletons_)
    {
        if (skeletonIndex == 0)
            p.setBrush(QBrush(Qt::red));
        else
            p.setBrush(QBrush(Qt::blue));

        // Draw bone lines
        QPen pen(Qt::SolidLine);
        pen.setWidth(3);
        pen.setBrush(p.brush()); 
        p.setPen(pen);

        helper_.ConnectBones(&p, imgRect, skeletonData["bone-head"], skeletonData["bone-shoulder-center"]);
        helper_.ConnectBones(&p, imgRect, skeletonData["bone-shoulder-center"], skeletonData["bone-spine"]);
        helper_.ConnectBones(&p, imgRect, skeletonData["bone-hip-center"], skeletonData["bone-spine"]);
        helper_.ConnectBones(&p, imgRect, skeletonData["bone-shoulder-left"], skeletonData["bone-shoulder-center"]);
        helper_.ConnectBones(&p, imgRect, skeletonData["bone-shoulder-right"], skeletonData["bone-shoulder-center"]);
        helper_.ConnectBones(&p, imgRect, skeletonData["bone-hip-center"], skeletonData["bone-hip-left"]);
        helper_.ConnectBones(&p, imgRect, skeletonData["bone-hip-center"], skeletonData["bone-hip-right"]);

        QList<QPair<QString, QString> > bonePairs;
        bonePairs << QPair<QString, QString>("hip", "knee");
        bonePairs << QPair<QString, QString>("ankle", "knee");
        bonePairs << QPair<QString, QString>("ankle", "foot");
        bonePairs << QPair<QString, QString>("shoulder", "elbow");
        bonePairs << QPair<QString, QString>("wrist", "elbow");
        bonePairs << QPair<QString, QString>("wrist", "hand");

        QStringList sides;
        sides << "-left" << "-right";

        foreach(QString side, sides)
            for(int i=0; i < bonePairs.length(); ++i)
                helper_.ConnectBones(&p, imgRect, skeletonData["bone-" + bonePairs[i].first + side], skeletonData["bone-" + bonePairs[i].second + side]);

        // Draw bone positions
        pen.setWidth(1);
        pen.setBrush(p.brush()); 
        p.setPen(pen);

        foreach(QString key, skeletonData.keys())
        {
            if (!key.startsWith("bone-"))
                continue;

            QVariant value = skeletonData[key];
            if (!value.canConvert<QVector4D>())
                continue;

            QVector4D vec = qvariant_cast<QVector4D>(value);
            vec *= 200;
            QPointF pos = vec.toPointF();

            if (key == "bone-head")
                p.drawEllipse(imgRect.center() + pos, 15, 15);
            else
                p.drawEllipse(imgRect.center() + pos, 4, 4);
        }

        skeletonIndex++;
    }
    p.end();

    skeletonPreviewLabel_->resize(imgSize);
    skeletonPreviewLabel_->show();
    skeletonPreviewLabel_->setPixmap(QPixmap::fromImage(manipImage.mirrored(false, true))); // Need to do a vertical flip for some reason
}

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

POCO_BEGIN_MANIFEST(IModule)
    POCO_EXPORT_CLASS(KinectModule)
POCO_END_MANIFEST

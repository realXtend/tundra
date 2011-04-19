// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "CameraInputModule.h"
#include "CameraAPI.h"

CameraAPI::CameraAPI(QObject *parent, Foundation::Framework *framework) :
    QObject(parent),
    framework_(framework),
    hasDevice_(false)
{
}

CameraAPI::~CameraAPI()
{
}

// Public

bool CameraAPI::HasDevice() 
{ 
    return hasDevice_; 
}

const QImage &CameraAPI::CurrentFrame()
{
    return currentFrame_;
}

// Protected

void CameraAPI::SetEnabled(bool enabled)
{
    hasDevice_ = enabled;
}

void CameraAPI::SetFrame(const QImage &frame)
{
    currentFrame_ = frame;
    emit frameUpdate(currentFrame_);
}
// For conditions of distribution and use, see copyright notice in license.txt

#include "IDevice.h"

IDevice::IDevice(const QString &name) :
    name_(name),
    running_(false),
    interfaceType_("IDevice")
{
}

IDevice::~IDevice()
{
    emit Destroyed();
}

void IDevice::Start()
{
    if (IsRunning())
        return;

    running_ = InternalStart();
    if (running_)
        emit Started();
}

void IDevice::Stop()
{
    if (!IsRunning())
        return;

    running_ = !InternalStop();
    if (!running_)
        emit Stopped();
}

bool IDevice::IsRunning()
{
    return running_;
}

QString IDevice::Name()
{
    return name_;
}

QString IDevice::InterfaceType()
{
    return interfaceType_;
}

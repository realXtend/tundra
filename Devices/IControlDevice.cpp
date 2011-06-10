// For conditions of distribution and use, see copyright notice in license.txt

#include "IControlDevice.h"

IControlDevice::IControlDevice(const QString &name) :
    IDevice(name)
{
    SetInterfaceType("IControlDevice");
}

IControlDevice::~IControlDevice()
{
}

AxisData::AxisData( int axi, float val ) : 
    axis(axi), value(val)
{
}

AxisData::~AxisData()
{
}

int AxisData::GetAxis()
{
    return axis;
}

float AxisData::GetValue()
{
    return value;
}

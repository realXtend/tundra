// For conditions of distribution and use, see copyright notice in license.txt

#include "DevicesAPI.h"
#include "IDevice.h"
#include "IPositionalDevice.h"

#include "Framework.h"
#include "FrameAPI.h"

#include "LoggingFunctions.h"
DEFINE_POCO_LOGGING_FUNCTIONS("Devices")

DevicesAPI::DevicesAPI(Foundation::Framework *framework) :
    QObject(framework),
    framework_(framework),
    doCleanup_(false)
{
    if (!connect(framework_->Frame(), SIGNAL(Updated(float)), SLOT(Update(float))))
        LogError("Could not connect to FrameAPI for update signals! Device updates are disabled.");
}

DevicesAPI::~DevicesAPI()
{
}

// Public

bool DevicesAPI::RegisterDevice(IDevice *device)
{
    DeviceCleanup();

    QString deviceNameLower = device->Name().toLower();
    if (deviceNameLower.isEmpty())
        return false;

    if (!devices.contains(deviceNameLower))
    {
        if (device->InternalInitialize())
        {
            devices[deviceNameLower] = QPointer<IDevice>(device);
            emit DeviceRegistered(deviceNameLower, device);
            connect(device, SIGNAL(Destroyed()), SLOT(OnDeviceDestroyed()));
            return true;
        }
        else
            LogWarning("Device '" + device->Name() + "' did not initialize properly, denying registration.");
    }
    else
        LogWarning("Device '" + device->Name() + "' already registered.");
    
    return false;
}

IDevice *DevicesAPI::GetDevice(const QString &name)
{
    DeviceCleanup();

    QString deviceNameLower = name.toLower();
    if (devices.contains(deviceNameLower))
    {
        IDevice* device = devices[deviceNameLower].data();
        if (device)
            return device;
    }
    return 0;
}

IDevice *DevicesAPI::CreateAndRegisterDevice(const QString &name)
{
    DeviceCleanup();

    QString deviceNameLower = name.toLower();
    if (devices.contains(deviceNameLower))
        return 0;

    IDevice *device = new IDevice(deviceNameLower);
    
    if (RegisterDevice(device))
    {
        device->setParent(this);
        return device;
    }
    else
    {
        delete device;
        return 0;
    }
}

IPositionalDevice *DevicesAPI::CreateAndRegisterPositionalDevice(const QString &name)
{
    DeviceCleanup();

    QString deviceNameLower = name.toLower();
    if (devices.contains(deviceNameLower))
        return 0;

    IPositionalDevice *device = new IPositionalDevice(deviceNameLower);
    
    if (RegisterDevice(device))
    {
        device->setParent(this);
        return device;
    }
    else
    {
        delete device;
        return 0;
    }
}

QStringList DevicesAPI::GetAllDeviceNames()
{
    DeviceCleanup();

    QStringList deviceNames(devices.keys());
    return deviceNames;
}

// Private

void DevicesAPI::DeviceCleanup()
{
    doCleanup_ = false;

    QStringList deadDevices;
    foreach(QPointer<IDevice> devicePtr, devices.values())
    {
        if (!devicePtr.data())
            deadDevices.append(devices.key(devicePtr));
    }
    foreach(QString deviceName, deadDevices)
    {
        devices.remove(deviceName);
        emit DeviceDestroyed(deviceName);
    }
}

void DevicesAPI::OnDeviceDestroyed()
{
    // We need to do delayed cleanup as the device ptr is still
    // valid here. The signal is emitted in its dtor.
    doCleanup_ = true;
}

void DevicesAPI::Update(float frametime)
{
    if (doCleanup_)
        DeviceCleanup();

    // Update running devices
    foreach(QPointer<IDevice> devicePtr, devices.values())
    {
        if (devicePtr.data())
            if (devicePtr->IsRunning())
                devicePtr->InternalUpdate(frametime);
    }
}

// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Devices_DeviceAPI_h
#define incl_Devices_DeviceAPI_h

#include <QObject>
#include <QString>
#include <QStringList>
#include <QHash>
#include <QPointer>

#include "DevicesFwd.h"

namespace Foundation { class Framework; }

typedef QHash<QString, QPointer<IDevice> > DeviceMap;

/*! DevicesAPI provides registering and getting devices. These devices emit data signals depending on what hardware device they are reading.
    See more from IDevice what you can do with devices that the DeviceAPI gives you.

    See also \ref device_api_page "DeviceAPI: Implementing and using a device with code examples"
*/
class DevicesAPI : public QObject
{

Q_OBJECT

public:
    DevicesAPI(Foundation::Framework *framework);
    virtual ~DevicesAPI();

public slots:
    /// Registers a new device to DeviceAPI.
    /// \note Devices must have a unique name, if you register a knows device this function will print warning and do nothing.
    /// \param Device* The device to be registered
    /// \return Bool True if succesfully registered, false otherwise.
    bool RegisterDevice(IDevice *device);

    /// Get a device by name.
    /// \param QString Name of the device.
    /// \return IDevice* Returns device pointer if found with name, otherwiwe 0.
    IDevice *GetDevice(const QString &name);

    /// Create and register a device new device with name. DeviceAPI takes ownership of the created device and releases its memory on shutdown.
    /// \note Convinience function for scripting languages so they don't have to inherit IDevice in their implementation. Prefer using inheriting and RegisterDevice() on C++.
    /// \return IDevice* Created device. If 0 the device was not creted and registered.
    IDevice *CreateAndRegisterDevice(const QString &name);

    /// Create and register a positional device new device with name. DeviceAPI takes ownership of the created device and releases its memory on shutdown.
    /// \note Convinience function for scripting languages so they don't have to inherit IPositionalDevice in their implementation. Prefer using inheriting and RegisterDevice() on C++.
    /// \return IPositionalDevice* Created device. If 0 the device was not created and registered.
    IPositionalDevice *CreateAndRegisterPositionalDevice(const QString &name);

    /// Get all the registered device names. You can get a device from GetDevice() with the names are returned.
    /// \return QStringList List of device names.
    QStringList GetAllDeviceNames();

private slots:
    /// Cleans dead QPointer<IDevice> from the device map.
    void DeviceCleanup();

    /// Handler for device destroyed.
    void OnDeviceDestroyed();

    /// Private slot that calls update to all the registered devices.
    /// \param float Frame time.
    void Update(float frametime);

signals:
    /// Emitted when a new device is registered.
    /// \param QString Device name.
    /// \param IDevice* Device object pointer.
    void DeviceRegistered(const QString &deviceName, IDevice *device);

    /// Emitted when a device is destroyed
    /// \param QString Device name.
    void DeviceDestroyed(const QString &deviceName);

private:
    /// Framework ptr.
    Foundation::Framework *framework_;

    /// Devices map.
    DeviceMap devices;

    /// Boolean that triggers cleanup.
    bool doCleanup_;
    
};

#endif
// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Devices_IDevice_h
#define incl_Devices_IDevice_h

#include <QObject>
#include <QString>
#include <QVariantMap>

/*! IDevice is a interface for any kind of device that provides data.

    IDevice is used in DevicesAPI, you can register and request devices by name. The device will emit data signals, depending on what the implementation is behind it.
    The most general signal is declared here on IDevice that is DataEvent(const QVariantMap &data) that emits raw data without specifying what it is. The content
    of the data needs to be interpreted from the maps key names that a QStrins, eg. key could be head_bone_position and the QVariant could be QVector3D.

    There are other interfaces that specify more detailed signals for certain type of devices, like IPositionalDevice that will emit position and orientation changes,
    but it can also utilize the generic DataEvent for sending additional data. You can always try to connect to the more detailed signals on a QObject, if they are not present
    the connection will fail. Or you can do a cast if you are in c++ and check if the device implements IPositionalDevice. At minimum you are guaranteed to have DataEvent present.

    \note If you are implementing a device you must override some common functionality that a device has. InternalInitialize(), InternalStart(), 
    InternalStop(), InternalUpdate() and optionally SetSettings() and GetSettings() if your device supports settings.

    See also \ref device_api_page "DeviceAPI: Implementing and using a device with code examples"

    Inherited by IPositionalDevice.
*/
class IDevice : public QObject
{

Q_OBJECT

Q_PROPERTY(QString name READ Name)

public:
    friend class DevicesAPI;

    /// IDevice constructor.
    IDevice(const QString &name);

    /// IDevice deconstructor.
    /// \note The member signal Destroyed is emitted inside this function.
    ~IDevice();

public slots:
    /// Start the device.
    void Start();

    /// Stop the device.
    void Stop();

    /// Get if device is running.
    /// \return bool True if device is running, false otherwise.
    bool IsRunning();

    /// Get the name of the device.
    /// \name QString The name of the device.
    QString Name();

    /// Get the interface type of the device as a string.
    /// \note This is mostly for scripting languages to determine type of the interface so it can connect to the correct signals.
    /// \return QString Type of the device, eg. "IDevice", "IPositonalDevice".
    QString InterfaceType();

    /// Set settings for the device. All devices may not support settings, you can use GetSettings for a map that has the current settings available.
    /// \param QVariantMap. QVariantMap is a typedef of QMap<QString, QVariant>. QString (the key) is name of settings, QVariant (the value) is the setting value.
    virtual void SetSettings(QVariantMap settings) {}

    /// Get settings for the device. All device may not support settings, if you get a empty map back there are no settings.
    /// \return QVariantMap. QVariantMap is a typedef of QMap<QString, QVariant>. QString (the key) is name of settings, QVariant (the value) is the setting value.
    virtual QVariantMap GetSettings() { return QVariantMap(); }

signals:
    /// Signal to notify when device was started.
    void Started();

    /// Signal to notify when device was stopped.
    void Stopped();

    /// Generic data event for emitting data.
    /// \note Not all devices use this generic data signal, they might emit device specific signals.
    /// \param QVariantMap. QVariantMap is a typedef of QMap<QString, QVariant>. QString (the key) is name of data variable, QVariant (the value) is the data.
    /// You can query QVariant::Type() for the data type if needed.
    void DataEvent(const QVariantMap &data);

    /// Signal will be emitted when this device is destroyed. Don't use this device after you receive this signal.
    void Destroyed();

protected:
    /// Initialise the device. If this function returns false, the device is not added to the registered devices map. Called from IDevice::RegisterDevice().
    /// \note There is no InternalUninitialize. Uninitialize your device in the deconstructor as usual, when IDevice::~IDevice() is invoked it will automatically emit Destroyed() signal for your device.
    /// \return Bool Return true if initializing was successful, false otherwise.
    virtual bool InternalInitialize() { return true; }

    /// Start the device, internal implementation. Called from IDevice::Start().
    /// \return Bool Return true if start was successful, false otherwise.
    virtual bool InternalStart() { return true; }

    /// Stop the device, internal implementation. Called from IDevice::Stop().
    /// \return Bool Return true if stop was successful, false otherwise.
    virtual bool InternalStop() { return true; }

    /// Update device I/O handling and emit data signals. Called from IDevice::Update() if this device is running.
    /// @param frametime Frame time.
    virtual void InternalUpdate(float frametime) {};

    /// Setter for the interface type. Called from I*Device implementations.
    void SetInterfaceType(const QString &interfaceType) { interfaceType_ = interfaceType; }

private:
    /// Name of the device. Can be accessed with IDevice::Name().
    QString name_;

    /// Interface type of the device.  Can be accessed with IDevice::InterfaceType().
    QString interfaceType_;

    /// Boolean for tracking if device is running. Can be accessed with IDevice::IsRunning().
    bool running_;
};

#endif
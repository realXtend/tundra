// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Devices_IControlDevice_h
#define incl_Devices_IControlDevice_h

#include "IDevice.h"
#include <QList>

/// AxisData is a event data class used with IControlDevice::AxisEvent.
class AxisData : public QObject
{
Q_OBJECT

/*! Axis of the movement. The implementing device will decide what 0..N axis actually means.
    Usually x = 0, y = 1 for a conventional joystick. They might be something else for non analog gamepads that
    have eg. 8 locked directions where it can be pressed. It all depends on the implementing code, so hopefully its documented.
*/
Q_PROPERTY(int axis READ GetAxis)

/// Value of the axis, should usually be >= -1.0 <= 1.0.
Q_PROPERTY(float value READ GetValue)

public:
    /// AxisData constructor.
    /// \param int Axis.
    /// \param float Value.
    AxisData(int axi, float val);

    /// AxisData deconstructor.
    virtual ~AxisData();

public slots:
    /// Get axis of this event.
    /// \return int Axis.
    int GetAxis();
    
    /// Get value of this event.
    /// \return float Value.
    float GetValue();

private:
    int axis;
    float value;
};

/*! IControlDevice is a interface for control devices like joysticks, wiimote etc. It declares specific signals for providing control/movement data.
    \note See IDevice on what protected functionality you need to override to implement a working device that you can use with DevicesAPI.

    Inherits IDevice.

    See also \ref device_api_page "DeviceAPI: Implementing and using a device with code examples"
*/
class IControlDevice : public IDevice
{

Q_OBJECT

public:
    /// IControlDevice constructor.
    IControlDevice(const QString &name);

    /// IControlDevice deconstructor.
    ~IControlDevice();

signals:
    /// AxisEvent with AxisData.
    /// \param QList<AxisData*> Event data objects in a QList.
    void AxisEvent(QList<AxisData*> aEvents);

    /// ButtonPressEvent, when button is pressed down. Optionally gives force, eg. for analog trigger buttons.
    /// \param QString Button name
    /// \param float Force of button press, defaults to 1.0.
    void ButtonPressEvent(QString buttonName, float force = 1.0f);

    /// ButtonReleaseEvent, when button is released.
    /// \param QString Button name
    void ButtonReleaseEvent(QString buttonName);

};

#endif


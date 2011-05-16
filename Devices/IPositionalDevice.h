// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Devices_IPositionalDevice_h
#define incl_Devices_IPositionalDevice_h

#include "IDevice.h"

#include "Vector3D.h"
#include "Quaternion.h"

/*! IPositionalDevice is a interface for positional data devices. It declares a specific signals for providing positional data.
    \note See IDevice on what protected functionality you need to override to implement a working device that you can use with DevicesAPI.

    Inherits IDevice.

    See also \ref device_api_page "DeviceAPI: Implementing and using a device with code examples"
*/
class IPositionalDevice : public IDevice
{

Q_OBJECT

public:
    /// IPositionalDevice constructor.
    IPositionalDevice(const QString &name);

    /// IPositionalDevice deconstructor.
    ~IPositionalDevice();

signals:
    /// PositionEvent sends data about positional changes of objects.
    /// \param QString Object name/id.
    /// \param Vector3df Position of the object.
    void PositionEvent(const QString &id, const Vector3df &position);

    /// OrientationEvent sends data about orientation changes of objects.
    /// \param QString Object name/id.
    /// \param Quaternion Orientation of the object.
    void OrientationEvent(const QString &id, const Quaternion &orientation);

};

#endif
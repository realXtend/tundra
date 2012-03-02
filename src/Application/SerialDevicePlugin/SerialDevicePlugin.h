// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "IModule.h"
#include <QString>

#include "qextserialport/qextserialport.h"

class QScriptEngine;

class QSerialPortInfo : public QObject
{

Q_OBJECT
Q_PROPERTY(QString portName READ PortName)
Q_PROPERTY(QString physicalName READ PhysicalName)
Q_PROPERTY(QString friendlyName READ FriendlyName)
Q_PROPERTY(QString enumName READ EnumName)
Q_PROPERTY(int vendorID READ VendorID)
Q_PROPERTY(int productID READ ProductID)

public:
    QSerialPortInfo() {}

    QString portName;
    QString physicalName;
    QString friendlyName;
    QString enumName;
    int vendorID;
    int productID;

    QString PortName() { return portName; }
    QString PhysicalName() { return physicalName; }
    QString FriendlyName() { return friendlyName; }
    QString EnumName() { return enumName; }
    int VendorID() { return vendorID; }
    int ProductID() { return productID; }

public slots:
    QString toString() { return QString("portName="+portName+",physicalName="+physicalName+",friendlyName="+friendlyName+
        ",enumName="+enumName+",vendorID=%1,productID=%2").arg(vendorID).arg(productID); }
};

class SerialDevicePlugin : public IModule
{

Q_OBJECT

public:
    /// Constructor.
    SerialDevicePlugin();

    /// Deconstructor.
    virtual ~SerialDevicePlugin();

    /// IModule override.
    void Initialize();

    /// IModule override.
    /// @note Frees all open devices from openDevices_ and port infos from serialInfos_.
    void Uninitialize();

public slots:
    /// Returns all available serial port information as a list. You can use the portName properties to call CreateDevice.
    QList<QSerialPortInfo*> PortInfoList();

    /// Creates a serial port device that inherits QIODevice, user QIODevice API to open/read/write the device after this call.
    /** @note Remember to close the device with CloseDevice */
    QextSerialPort *CreateDevice(QString portName, QextSerialPort::QueryMode mode = QextSerialPort::EventDriven);

    /// Creates a serial port device that inherits QIODevice, user QIODevice API to open/read/write the device after this call.
    /** Takes all configurable parameters and sets them to the created device.
        @note You can use the simpler CreateDevice override and use the slots to set the settings. 
        @note Remember to close the device with CloseDevice */
    QextSerialPort *CreateDevice(QString portName, BaudRateType baudRate, DataBitsType dataBits, ParityType parity,
                               StopBitsType stopBits, FlowType flowControl, QextSerialPort::QueryMode mode = QextSerialPort::EventDriven);
    //Open port
    bool OpenDevice(QextSerialPort *device);

    //Write To Port
    void WriteToDevice(QextSerialPort *device, QString command);

    //Read From Port
    QString ReadFromDevice(QextSerialPort *device);

    // Closes a device. Do not use the device after calling this as the ptr will be freed.
    bool CloseDevice(QString portName);

    // Closes a device. Do not use the device after calling this as the ptr will be freed.
    bool CloseDevice(QextSerialPort *device);

private slots:
    void RefreshPortInfo();
    void OnScriptEngineCreated(QScriptEngine *engine);
    
private:
    QString LC;
    QList<QSerialPortInfo*> serialInfos_;
    QList<QextSerialPort*> openDevices_;
};

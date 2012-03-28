// For conditions of distribution and use, see copyright notice in LICENSE

#include "SerialDevicePlugin.h"
#include "Framework.h"
#include "CoreDefines.h"
#include "LoggingFunctions.h"

#include "qextserialport/qextserialenumerator.h"

#include "SerialDeviceScriptTypeDefines.h"
#include "JavascriptModule.h"

SerialDevicePlugin::SerialDevicePlugin() :
    IModule("SerialDevicePlugin"),
    LC("[SerialDevices]: ")
{
}

SerialDevicePlugin::~SerialDevicePlugin()
{
}

void SerialDevicePlugin::Initialize()
{
    RegisterSerialDevicePluginMetaTypes();

    // Register custom QObject and enum meta types to created script engines.
    JavascriptModule *javascriptModule = framework_->GetModule<JavascriptModule>();
    if (javascriptModule)
        connect(javascriptModule, SIGNAL(ScriptEngineCreated(QScriptEngine*)), SLOT(OnScriptEngineCreated(QScriptEngine*)));
    else
        LogWarning(LC + "JavascriptModule not present, SerialDevicePlugin usage from scripts will be limited!");

    framework_->RegisterDynamicObject("serialdevices", this);
}

void SerialDevicePlugin::Uninitialize()
{
    foreach(QSerialPortInfo *portInfoIter, serialInfos_)
        SAFE_DELETE(portInfoIter);
    serialInfos_.clear();

    foreach(QextSerialPort *deviceIter, openDevices_)
    {
        if (deviceIter->isOpen())
            deviceIter->close();
        SAFE_DELETE(deviceIter);
    }
    openDevices_.clear();
}

void SerialDevicePlugin::OnScriptEngineCreated(QScriptEngine *engine)
{
    RegisterSerialDevicePluginMetaTypes(engine);
}

QList<QSerialPortInfo*> SerialDevicePlugin::PortInfoList()
{
    RefreshPortInfo();
    return serialInfos_;
}

QextSerialPort *SerialDevicePlugin::CreateDevice(QString portName, QextSerialPort::QueryMode mode)
{
    QextSerialPort *device = new QextSerialPort(portName, mode);
    if (!device)
    {
        LogError(LC + "Failed to create new device with port name '" + portName + "'");
        return 0;
    }
    
    device->setBaudRate(BAUD4800);
    device->setFlowControl(FLOW_OFF);
    device->setParity(PAR_NONE);
    device->setDataBits(DATA_8);
    device->setStopBits(STOP_2);
    device->setTimeout(500);
    device->flush();
    openDevices_.push_back(device);
    return device;
}

QextSerialPort *SerialDevicePlugin::CreateDevice(QString portName, BaudRateType baudRate, DataBitsType dataBits, ParityType parity,
    StopBitsType stopBits, FlowType flowControl, QextSerialPort::QueryMode mode)
{
    QextSerialPort *device = CreateDevice(portName, mode);
    if (!device)
        return 0;
        
    device->setBaudRate(baudRate);
    device->setDataBits(dataBits);
    device->setParity(parity);
    device->setStopBits(stopBits);
    device->setFlowControl(flowControl);
    device->setTimeout(500);
    device->flush();
    return device;
}

bool SerialDevicePlugin::OpenDevice(QextSerialPort *device)
{
    if (device)
    {
        device->open(QIODevice::ReadWrite | QIODevice::Unbuffered);
        return device->isOpen();
    }
    else
    {
        LogError(LC + "OpenDevice called with null device ptr!");
        return false;
    }
}

bool SerialDevicePlugin::CheckDevice(QextSerialPort *device)
{
    if (!device)
    {
        LogError(LC + "CheckDevice called with null device ptr!");
        return false;
    }
    return device->isOpen();
}

void SerialDevicePlugin::WriteToDevice(QextSerialPort *device, QString command)
{
    try
    {
        if (device)
        {
            if (device->isOpen())
            {
                device->flush();
                device->write(command.toAscii(), command.length());
            }
            else
                LogError(LC + "Device is not open!");
        }
        else
            LogError(LC + "WriteToDevice called with null device ptr!");
    }
    catch(char *str)
    {
        LogError(LC + *str);
    }
 }

QString SerialDevicePlugin::ReadFromDevice(QextSerialPort *device)
{
    try
    {
        if (!device)
        {
            LogError(LC + "ReadFromDevice called with null device ptr!");
            return "";
        }

        if (device->isOpen())
        {
            qint64 numBytes;

            numBytes = device->bytesAvailable();
            //if (numBytes > 1024)
                //numBytes = 1024;

            QByteArray i = device->read(numBytes);
            QString msg = i;

            return msg;
        } else
            return "";
    }
    catch(char *str)
    {
        LogError(LC + *str);
    }
}

bool SerialDevicePlugin::CloseDevice(QString portName)
{
    QextSerialPort *removeIter = 0;
    foreach(QextSerialPort *portIter, openDevices_)
    {
        if (portIter->portName() == portName)
        {
            removeIter = portIter;
            break;
        }
    }
    if (removeIter)
        return CloseDevice(removeIter);
    else
        return false;
}

bool SerialDevicePlugin::CloseDevice(QextSerialPort *device)
{
    if (!device)
    {
        LogError(LC + "CloseDevice called with null device ptr!");
        return false;
    }
    
    int removeIndex = -1;
    for(int i=0; i<openDevices_.size(); i++)
    {
        if (openDevices_[i] == device)
        {
            removeIndex = i;
            break;
        }
    }
    if (removeIndex != -1)
        openDevices_.removeAt(removeIndex);

    if (device->isOpen())
        device->close();
    SAFE_DELETE(device);
    return true;
}

void SerialDevicePlugin::RefreshPortInfo()
{
    foreach(QSerialPortInfo *portInfoIter, serialInfos_)
        SAFE_DELETE(portInfoIter);
    serialInfos_.clear();

    QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();
    foreach(QextPortInfo infoIter, ports)
    {
        QSerialPortInfo *info = new QSerialPortInfo();
        info->portName = infoIter.portName;
        info->friendlyName = infoIter.friendName;
        info->physicalName = infoIter.physName;
        info->enumName = infoIter.enumName;
        info->vendorID = infoIter.vendorID;
        info->productID = infoIter.productID;
        serialInfos_ << info;
    }
}

extern "C"
{
    DLLEXPORT void TundraPluginMain(Framework *fw)
    {
        Framework::SetInstance(fw); // Inside this DLL, remember the pointer to the global framework object.
        IModule *module = new SerialDevicePlugin();
        fw->RegisterModule(module);
    }
}

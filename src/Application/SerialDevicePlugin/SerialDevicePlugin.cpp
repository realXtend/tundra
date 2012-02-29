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
    openDevices_.push_back(device);
    return device;
}

QextSerialPort *SerialDevicePlugin::CreateDevice(QString portName, BaudRateType baudRate, DataBitsType dataBits, ParityType parity,
    StopBitsType stopBits, FlowType flowControl, QextSerialPort::QueryMode mode)
{
    QextSerialPort *device = CreateDevice(portName, mode);
    device->setBaudRate(baudRate);
    device->setDataBits(dataBits);
    device->setParity(parity);
    device->setStopBits(stopBits);
    device->setFlowControl(flowControl);
    return device;
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

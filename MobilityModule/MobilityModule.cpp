/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   MobilityModule.cpp
 *  @brief  Stub implementation of the MobilityModule which is supposed to gather data from various
 *          sources and pass it on for other modules.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "ModuleManager.h"

#include "MobilityModule.h"

#include "EventManager.h"
#include "MemoryLeakCheck.h"

#include <QMapIterator>
#include <QFlags>

Q_DECLARE_METATYPE(MobilityModule*);
Q_DECLARE_METATYPE(MobilityModule::DeviceFeature);
Q_DECLARE_METATYPE(MobilityModule::NetworkState);
Q_DECLARE_METATYPE(MobilityModule::NetworkMode);
Q_DECLARE_METATYPE(MobilityModule::ScreenState);

template<typename T> QScriptValue toScriptValueEnum(QScriptEngine *engine, const T &s)
{
    return QScriptValue((int)s);
}

template<typename T> void fromScriptValueEnum(const QScriptValue &obj, T &s)
{
    s = static_cast<T>(obj.toInt32());
}

QTM_USE_NAMESPACE

std::string MobilityModule::type_name_static_ = "Mobility";

MobilityModule::MobilityModule() :
    IModule(type_name_static_),
    system_info_(new QSystemInfo(this)),
    system_device_info_(new QSystemDeviceInfo(this)),
    system_network_info_(new QSystemNetworkInfo(this)),
    system_display_info_(0),
    network_configuration_manager_(new QNetworkConfigurationManager(this))
{
}

MobilityModule::~MobilityModule()
{
}

void MobilityModule::PreInitialize()
{
}

void MobilityModule::Initialize()
{
    LogInfo(Name() + " initializing...");

    /// \todo move these to a separate source?
    // Map corresponding values from QSystemInfo::Feature to MobilityModule::DeviceFeature
    deviceFeatureMap_[QSystemInfo::BluetoothFeature] = MobilityModule::BluetoothFeature;
    deviceFeatureMap_[QSystemInfo::CameraFeature] = MobilityModule::CameraFeature;
    deviceFeatureMap_[QSystemInfo::FmradioFeature] = MobilityModule::FmradioFeature;
    deviceFeatureMap_[QSystemInfo::IrFeature] = MobilityModule::IrFeature;
    deviceFeatureMap_[QSystemInfo::LedFeature] = MobilityModule::LedFeature;
    deviceFeatureMap_[QSystemInfo::MemcardFeature] = MobilityModule::MemcardFeature;
    deviceFeatureMap_[QSystemInfo::UsbFeature] = MobilityModule::UsbFeature;
    deviceFeatureMap_[QSystemInfo::VibFeature] = MobilityModule::VibFeature;
    deviceFeatureMap_[QSystemInfo::WlanFeature] = MobilityModule::WlanFeature;
    deviceFeatureMap_[QSystemInfo::SimFeature] = MobilityModule::SimFeature;
    deviceFeatureMap_[QSystemInfo::LocationFeature] = MobilityModule::LocationFeature;
    deviceFeatureMap_[QSystemInfo::VideoOutFeature] = MobilityModule::VideoOutFeature;
    deviceFeatureMap_[QSystemInfo::HapticsFeature] = MobilityModule::HapticsFeature;

    // Map corresponding values from QSystemNetworkInfo::NetworkStatus to MobilityModule::NetworkState
    networkStateMap_[QSystemNetworkInfo::UndefinedStatus] = MobilityModule::StateUndetermined;
    networkStateMap_[QSystemNetworkInfo::NoNetworkAvailable] = MobilityModule::StateDisconnected;
    networkStateMap_[QSystemNetworkInfo::Searching] = MobilityModule::StateConnecting;
    networkStateMap_[QSystemNetworkInfo::Connected] = MobilityModule::StateConnected;
    networkStateMap_[QSystemNetworkInfo::Roaming] = MobilityModule::StateRoaming;

    // Map corresponding values from QSystemNetworkInfo::NetworkMode to MobilityModule::NetworkMode
    networkModeMap_[QSystemNetworkInfo::UnknownMode] = MobilityModule::ModeUnknown;
    networkModeMap_[QSystemNetworkInfo::GsmMode] = MobilityModule::ModeGsm;
    networkModeMap_[QSystemNetworkInfo::CdmaMode] = MobilityModule::ModeCdma;
    networkModeMap_[QSystemNetworkInfo::WcdmaMode] = MobilityModule::ModeWcdma;
    networkModeMap_[QSystemNetworkInfo::WlanMode] = MobilityModule::ModeWlan;
    networkModeMap_[QSystemNetworkInfo::EthernetMode] = MobilityModule::ModeEthernet;
    networkModeMap_[QSystemNetworkInfo::BluetoothMode] = MobilityModule::ModeBluetooth;
    networkModeMap_[QSystemNetworkInfo::WimaxMode] = MobilityModule::ModeWimax;


    // Query platform capabilities from QSystemInfo and QSystemDeviceInfo
    features_.clear();
    QMapIterator<QSystemInfo::Feature, MobilityModule::DeviceFeature> iterator_(deviceFeatureMap_);
    while(iterator_.hasNext())
    {
        iterator_.next();
        features_.insert(iterator_.value(), system_info_->hasFeatureSupported(iterator_.key()));
    }

    QFlags<QSystemDeviceInfo::InputMethod> inputmethods_ = system_device_info_->inputMethodType();
    features_.insert(MobilityModule::KeyboardFeature, inputmethods_.testFlag(QSystemDeviceInfo::Keyboard));
    features_.insert(MobilityModule::SingleTouchFeature, inputmethods_.testFlag(QSystemDeviceInfo::SingleTouch));
    features_.insert(MobilityModule::MultiTouchFeature, inputmethods_.testFlag(QSystemDeviceInfo::MultiTouch));
    features_.insert(MobilityModule::MouseFeature, inputmethods_.testFlag(QSystemDeviceInfo::Mouse));


    // Set initial values for mobility related data
    battery_critical_ = false;
    SetBatteryCriticalValue(20);

    // Getter for networkstate isn't properly implemented in QtMobility 1.2. Use workaround for determining initial state.
    network_state_ = getNetworkState();
    network_mode_ = networkModeMap_.value(system_network_info_->currentMode());
    network_quality_ = 100; // Default return value

    usingBatteryHandler(system_device_info_->currentPowerState());
    batteryLevelHandler(system_device_info_->batteryLevel());


    connect(system_device_info_, SIGNAL(batteryLevelChanged(int)),
            this, SLOT(batteryLevelHandler(int)));
    connect(system_device_info_, SIGNAL(powerStateChanged(QSystemDeviceInfo::PowerState)),
            this, SLOT(usingBatteryHandler(QSystemDeviceInfo::PowerState)));


    connect(system_network_info_, SIGNAL(networkModeChanged(QSystemNetworkInfo::NetworkMode)),
            this, SLOT(networkModeHandler(QSystemNetworkInfo::NetworkMode)));
    connect(system_network_info_, SIGNAL(networkStatusChanged(QSystemNetworkInfo::NetworkMode,QSystemNetworkInfo::NetworkStatus)),
            this, SLOT(networkStateHandler(QSystemNetworkInfo::NetworkMode,QSystemNetworkInfo::NetworkStatus)));
    connect(system_network_info_, SIGNAL(networkSignalStrengthChanged(QSystemNetworkInfo::NetworkMode,int)),
            this, SLOT(networkQualityHandler(QSystemNetworkInfo::NetworkMode,int)));

    framework_->RegisterDynamicObject("mobility", this);
}

void MobilityModule::PostInitialize()
{
}

bool MobilityModule::HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData* data)
{
    return false;
}

void MobilityModule::Uninitialize()
{
}

void MobilityModule::Update(f64 frametime)
{
}

void MobilityModule::batteryLevelHandler(int batteryLevel)
{
    // Battery critical signal only gets emitted once when using battery power and battery level drops below set critical.
    if(using_battery_power_)
    {
        if(batteryLevel <= battery_critical_value_ && !battery_critical_)
        {
            battery_critical_ = true;
            LogWarning("Battery state critical");
            emit batteryLevelCritical();
        }
    }

    battery_level_ = batteryLevel;

    emit batteryLevelChanged(battery_level_);

    LogInfo("Battery level changed to: " + ToString(battery_level_));
}

void MobilityModule::usingBatteryHandler(QSystemDeviceInfo::PowerState powerState)
{
    if(powerState == QSystemDeviceInfo::BatteryPower)
    {
        using_battery_power_ = true;
        LogInfo("Power source changed to: Battery");
        batteryLevelHandler(system_device_info_->batteryLevel()); // Call batteryLevelHandler() to emit current batterylevel and to set battery_critical_ to correct state
    }
    else
    {
        using_battery_power_ = false;
        battery_critical_ = false;
        LogInfo("Power source changed to: Wall");
    }

    emit usingBattery(using_battery_power_);
}

void MobilityModule::networkStateHandler(QSystemNetworkInfo::NetworkMode mode, QSystemNetworkInfo::NetworkStatus status)
{
    // Qt Mobility sometimes emits status changed signal twice, check if the status actually changed.
    // Handle signal only if the state change affects current active network mode.
    if(network_state_ != networkStateMap_.value(status) && (network_mode_ == networkModeMap_.value(mode) || network_mode_ == MobilityModule::ModeUnknown))
    {
        if(networkStateMap_.contains(status))
            network_state_ = networkStateMap_.value(status);
        else
            network_state_ = MobilityModule::StateUndetermined;

        switch(network_state_)
        {
        case MobilityModule::StateUndetermined:
            LogInfo("Network state changed to: Undertermined");
            break;
        case MobilityModule::StateConnecting:
            LogInfo("Network state changed to: Connecting");
            break;
        case MobilityModule::StateConnected:
            LogInfo("Network state changed to: Connected");
            break;
        case MobilityModule::StateDisconnected:
            LogInfo("Network state changed to: Disconnected");
            break;
        case MobilityModule::StateRoaming:
            LogInfo("Network state changed to: Roaming");
            break;
        default:
            break;
        }

        emit networkStateChanged(network_state_);
    }
}


void MobilityModule::screenStateHandler(int screenState)
{
    // Unimplemented, see header for details.
}

void MobilityModule::networkModeHandler(QSystemNetworkInfo::NetworkMode networkMode)
{
    if(network_mode_ != networkModeMap_.value(networkMode))
    {
        if(networkModeMap_.contains(networkMode))
            network_mode_ = networkModeMap_.value(networkMode);
        else
            network_mode_ = MobilityModule::ModeUnknown;

        switch(network_mode_)
        {
        case MobilityModule::ModeUnknown:
            LogInfo("Networkmode changed to: Unkwnown");
            break;
        case MobilityModule::ModeGsm:
            LogInfo("Networkmode changed to: Gsm");
            break;
        case MobilityModule::ModeCdma:
            LogInfo("Networkmode changed to: Cdma");
            break;
        case MobilityModule::ModeWcdma:
            LogInfo("Networkmode changed to: Wcdma");
            break;
        case MobilityModule::ModeWlan:
            LogInfo("Networkmode changed to: Wlan");
            break;
        case MobilityModule::ModeEthernet:
            LogInfo("Networkmode changed to: Ethernet");
            break;
        case MobilityModule::ModeBluetooth:
            LogInfo("Networkmode changed to: Bluetooth");
            break;
        case MobilityModule::ModeWimax:
            LogInfo("Networkmode changed to: Wimax");
            break;
        default:
            LogDebug("Catched QSystemNetworkInfo::networkModeChanged() but was unable to determine the mode.");
            break;
        }

        emit networkModeChanged(network_mode_);
    }
}

void MobilityModule::networkQualityHandler(QSystemNetworkInfo::NetworkMode mode, int strength)
{
    if(network_quality_ != strength && network_mode_ == networkModeMap_.value(mode))
    {
        network_quality_ = strength;
        LogInfo("Network quality changed to: " + ToString(network_quality_));
        emit networkQualityChanged(network_quality_);
    }
}

int MobilityModule::GetBatteryLevel() const
{
    return battery_level_;
}

bool MobilityModule::GetUsingBattery() const
{
    return using_battery_power_;
}

bool MobilityModule::GetBatteryCritical() const
{
    return battery_critical_;
}

MobilityModule::NetworkState MobilityModule::GetNetworkState() const
{
    return network_state_;
}

MobilityModule::NetworkMode MobilityModule::GetNetworkMode() const
{
    return network_mode_;
}

MobilityModule::ScreenState MobilityModule::GetScreenState() const
{
    return MobilityModule::ScreenOn;
}

int MobilityModule::GetNetworkQuality() const
{
    return network_quality_;
}

void MobilityModule::SetBatteryCriticalValue(int criticalValue)
{
    if(criticalValue >= 0 && criticalValue <= 100)
        battery_critical_value_ = criticalValue;
}

bool MobilityModule::GetFeatureAvailable(MobilityModule::DeviceFeature feature) const
{
    if(features_.contains(feature))
        return features_.value(feature);
    else
        return false;
}

MobilityModule::NetworkState MobilityModule::getNetworkState()
{
    QList<QNetworkConfiguration> configs_ = network_configuration_manager_->allConfigurations(QNetworkConfiguration::Active);

    // QNetworkConfigurationManager counts interfaces as active configurations (atleast on Linux), these need to be removed.
    foreach(const QNetworkInterface iface, QNetworkInterface::allInterfaces())
    {
        for(int x = 0; x < configs_.size(); x++)
        {
            if(QString::compare(configs_.at(x).name(), iface.humanReadableName(), Qt::CaseInsensitive) == 0)
            {
                configs_.removeAt(x);
                x--; // Rest of the list has shifted one position towards zero, check the same position again.
            }
        }
    }

    // If one or more (non-interface) configurations are active, system is considered to be in online state.
    if(configs_.size() > 0)
        return MobilityModule::StateConnected;
    else
        return MobilityModule::StateDisconnected;
}

void MobilityModule::OnScriptEngineCreated(QScriptEngine* engine)
{
    qScriptRegisterMetaType(engine, toScriptValueEnum<MobilityModule::DeviceFeature>, fromScriptValueEnum<MobilityModule::DeviceFeature>);
    qScriptRegisterMetaType(engine, toScriptValueEnum<MobilityModule::NetworkState>, fromScriptValueEnum<MobilityModule::NetworkState>);
    qScriptRegisterMetaType(engine, toScriptValueEnum<MobilityModule::NetworkMode>, fromScriptValueEnum<MobilityModule::NetworkMode>);
    qScriptRegisterMetaType(engine, toScriptValueEnum<MobilityModule::ScreenState>, fromScriptValueEnum<MobilityModule::ScreenState>);
}

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

POCO_BEGIN_MANIFEST(IModule)
   POCO_EXPORT_CLASS(MobilityModule)
POCO_END_MANIFEST

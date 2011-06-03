// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_MobilityModule_MobilityModule_h
#define incl_MobilityModule_MobilityModule_h

#include "IModule.h"
#include "ModuleLoggingFunctions.h"

#include <QObject>
#include <QString>
#include <QList>

#include <QSystemInfo>
#include <QSystemDeviceInfo>
#include <QSystemNetworkInfo>
#include <QSystemDisplayInfo>

#include <QNetworkSession>
#include <QNetworkConfigurationManager>
#include <QNetworkConfiguration>
#include <QNetworkInterface>

#include <QtScript>

/// Needed for MOC to regocnize params in signals/slots to be the same.
/// Feel free to change if there's a better way to overcome this annoyance.
using QtMobility::QSystemInfo;
using QtMobility::QSystemDeviceInfo;
using QtMobility::QSystemNetworkInfo;
using QtMobility::QSystemDisplayInfo;


/// Acts as an proxy between mobility related data and rest of the modules.
/// Currently uses QtMobility as the only source of the information.
/// \note Functionality only tested on Linux platform. Some modifications
///       maybe needed in order to use the module on Windows/Mac because
///       of the status of the QtMobility project.
/// \todo Interface MCE daemon once it becomes part of MeeGo.
class MobilityModule : public QObject, public IModule
{
    Q_OBJECT
    Q_ENUMS(DeviceFeature)
    Q_ENUMS(NetworkState)
    Q_ENUMS(NetworkMode)
    Q_ENUMS(ScreenState)

public:
    enum DeviceFeature
    {
        BluetoothFeature = 0,
        CameraFeature = 1,
        FmradioFeature = 2,
        IrFeature = 3,
        LedFeature = 4,
        MemcardFeature = 5,
        UsbFeature = 6,
        VibFeature	= 7,
        WlanFeature	= 8,
        SimFeature	= 9,
        LocationFeature = 10,
        VideoOutFeature = 11,
        HapticsFeature = 12,
        KeyboardFeature = 13,
        SingleTouchFeature = 14,
        MultiTouchFeature = 15,
        MouseFeature = 16
    };

    enum NetworkState
    {
        StateUndetermined = 0,
        StateConnecting = 1,
        StateConnected = 2,
        StateDisconnected = 3,
        StateRoaming = 4
    };

    enum NetworkMode
    {
        ModeUnknown = 0,
        ModeGsm = 1,
        ModeCdma = 2,
        ModeWcdma = 3,
        ModeWlan = 4,
        ModeEthernet = 5,
        ModeBluetooth = 6,
        ModeWimax = 7,
    };

    enum ScreenState
    {
        ScreenUnknown = 0,
        ScreenOn = 1,
        ScreenDimmed = 2,
        ScreenOff = 3,
        ScreenSaver = 4
    };

    MODULE_LOGGING_FUNCTIONS

    /// Default constructor.
    MobilityModule();

    /// Destructor.
    ~MobilityModule();

    /// IModule override.
    void PreInitialize();

    /// IModule override.
    void Initialize();

    /// IModule override.
    void PostInitialize();

    /// IModule override.
    void Uninitialize();

    /// IModule override.
    void Update(f64 frametime);

    /// IModule override.
    bool HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData* data);

    /// Returns name of this module. Needed for logging.
    static const std::string &NameStatic() { return type_name_static_; }

private:

    /// QSystemNetworkInfo::networkStatus() has only been implemented for a few connection types
    /// in Qt Mobility 1.2. Changes in connection status propagate correctly for all connection types
    /// though. Use this workaround function to determine the initial connection status.
    MobilityModule::NetworkState getNetworkState();

    /// Type name of the module.
    static std::string type_name_static_;

    /// QSystemInfo, source of device capabilities related info
    QSystemInfo *system_info_;

    /// QSystemDeviceInfo, source of battery related information
    QSystemDeviceInfo *system_device_info_;

    /// QSystemNetworkInfo
    QSystemNetworkInfo *system_network_info_;

    /// QSystemDisplayInfo
    QSystemDisplayInfo *system_display_info_;
    
    /// QNetworkConfigurationManager, used to keep track of active network configurations
    QNetworkConfigurationManager *network_configuration_manager_;

    /// Map of available features
    QMap<MobilityModule::DeviceFeature, bool> features_;

    /// Current battery level
    int battery_level_;

    /// Current battery critical state
    bool battery_critical_;

    /// Value after which battery is considered to be in critical state
    int battery_critical_value_;

    /// Current power source (true for battery, false for wall)
    bool using_battery_power_;

    /// Current screen state
    ScreenState screen_state_;

    /// Current network mode
    NetworkMode network_mode_;

    /// Current network state
    NetworkState network_state_;

    /// Current network quality
    int network_quality_;

    /// Active network session
    QNetworkSession *network_session_;

    /// Mapping from QSystemNetworkInfo::NetworkStatus to MobilityModule::NetworkState
    QMap<QSystemNetworkInfo::NetworkStatus, MobilityModule::NetworkState> networkStateMap_;

    /// Mapping from QSystemNetworkInfo::NetworkMode to MobilityModule::NetworkMode
    QMap<QSystemNetworkInfo::NetworkMode, MobilityModule::NetworkMode> networkModeMap_;

    /// Mapping from QSystemInfo::Feature to MobilityModule::DeviceFeature
    QMap<QSystemInfo::Feature, MobilityModule::DeviceFeature> deviceFeatureMap_;

private slots:

    /// Handler for battery level information
    void batteryLevelHandler(int batteryLevel);

    /// Handler for power source information
    void usingBatteryHandler(QSystemDeviceInfo::PowerState powerState);

    /// Handler for network state information
    void networkStateHandler(QSystemNetworkInfo::NetworkMode mode, QSystemNetworkInfo::NetworkStatus status);

    /// Handler for screen state information
    /// \todo Implement when MCE daemon becomes part of MeeGo or QSystemDeviceInfo provides the required data.
    void screenStateHandler(int screenState);

    /// Handler for network mode information
    void networkModeHandler(QSystemNetworkInfo::NetworkMode networkMode);

    /// Handler for network signal strength
    /// \note Qt Mobility 1.2 doesn't properly implement signaling for network quality changes,
    /// so this is mainly for future use (we do get quality data when network mode changes though).
    void networkQualityHandler(QSystemNetworkInfo::NetworkMode mode, int strength);
    
public slots:

    /// Initialize datatypes for a script engine
    void OnScriptEngineCreated(QScriptEngine* engine);

    /// \return Current battery level percentage (0-100)
    int GetBatteryLevel() const;

    /// \return Is the system using battery or wall power (true for battery, false for wall)
    bool GetUsingBattery() const;

    /// \return Boolean for battery critical state
    /// \note Always returns false when on battery power
    bool GetBatteryCritical() const;

    /// \return Current network state as defined in MobilityModule::NetworkState
    MobilityModule::NetworkState GetNetworkState() const;

    /// \return Current network mode as defined in MobilityModule::NetworkMode
    MobilityModule::NetworkMode GetNetworkMode() const;

    /// \return Current network quality percentage (0-100)
    int GetNetworkQuality() const;

    /// \return Current screen state as defined in MobilityModule::ScreenState
    MobilityModule::ScreenState GetScreenState() const;

    /// Checks if platform has a feature available
    /// \param feature Feature defined in MobilityModule::DeviceFeature
    /// \return Boolean for availability of the given feature
    bool GetFeatureAvailable(MobilityModule::DeviceFeature feature) const;

    /// \param Critical level for battery power in percentage (1-100)
    void SetBatteryCriticalValue(int criticalValue);

signals:

    /// Emitted when power state changes from wall to battery or vice versa
    void usingBattery(bool state);

    /// Emitted when battery level changes
    /// \note Only emitted when using battery power
    void batteryLevelChanged(int batteryLevel);

    /// Emitted when battery level drops to critical
    /// \note Only emitted once when battery level drops below critical threshold
    /// \note Not emitted when on wall power (shouldn't happen anyways)
    void batteryLevelCritical();

    /// Emitted when network status changes (offline/online/roaming...)
    /// \note Not tested on ConnMan backend. With NetworkManager, only signals Connected and Disconnected states.
    void networkStateChanged(MobilityModule::NetworkState networkState);

    /// Emitted when network mode changes
    void networkModeChanged(MobilityModule::NetworkMode networkMode);

    /// Emitted when network quality changes
    /// \note Currently unimplemented
    void networkQualityChanged(int networkQuality);

    /// Emitted when screen status changes
    /// \note Currently unimplemented
    void screenStateChanged(MobilityModule::ScreenState screenState);
};


#endif // incl_MobilityModule_MobilityModule_h

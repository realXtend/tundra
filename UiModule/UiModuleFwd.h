// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_Fwd_h
#define incl_UiModule_Fwd_h

class QGraphicsView;
class QGraphicsScene;
class QGraphicsWidget;
class QGraphicsProxyWidget;

namespace CoreUi
{
    class UiStateMachine;
    
    class AnchorLayoutManager;
    class ControlPanelManager;
    class UiConsoleManager;
    class MenuManager;

    class MainPanel;
    class SettingsWidget;
    class CommunicationWidget;
    class ConsoleProxyWidget;
}

namespace UiServices
{
    class InworldSceneController;
    class NotificationManager;
    
    class UiProxyWidget;
    class UiWidgetProperties;    
}

namespace Ether
{
    namespace Logic
    {
        class EtherLogic;
        class EtherSceneController;
    }
    namespace View
    {
        class InfoCard;
        class EtherScene;
        class VerticalMenu;
        class ControlProxyWidget;
        class ActionProxyWidget;
        class EtherMenu;
    }
    namespace Data
    {
        class DataManager;
        class AvatarInfo;
        class WorldInfo;
        class OpenSimAvatar;
        class RealXtendAvatar;
        class OpenSimWorld;
    }
}

#endif

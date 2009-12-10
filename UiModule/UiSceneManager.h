// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_UiSceneManager_h
#define incl_UiModule_UiSceneManager_h

#include "Foundation.h"
#include "UiModuleApi.h"

#include <QObject>
#include <QRectF>
#include <QGraphicsItem>
#include <QGraphicsLinearLayout>
#include <QGraphicsWidget>
#include <QList>

namespace CoreUi
{
    class MainPanel;
}

namespace UiServices
{
    class UiProxyWidget;
    class UiWidgetProperties;

    class UI_MODULE_API UiSceneManager : public QObject
    {

    Q_OBJECT

    public:
        UiSceneManager(Foundation::Framework *framework, QGraphicsView *ui_view);
        ~UiSceneManager();

    public slots:
        
        //! Adds a Qt Widget to the current scene, returns the added QGraphicsProxyWidget
        UiProxyWidget* AddWidgetToCurrentScene(QWidget *widget);

        //! Adds a Qt Widget to the current scene with Naali widget properties, returns the added QGraphicsProxyWidget
		UiProxyWidget* AddWidgetToCurrentScene(QWidget *widget, const UiServices::UiWidgetProperties &widget_properties);

        //! Adds a already created UiProxyWidget into the scene, used for 3D to canvas swaps
        bool AddProxyWidget(UiServices::UiProxyWidget *widget);

        //! Remove a proxy widget from scene if it exist there
        void RemoveProxyWidgetFromCurrentScene(const UiProxyWidget *widget);

        //! Get the inworld controls
        CoreUi::MainPanel *GetMainPanel() { return main_panel_; }

        //! Gets all present proxy widgets from scene
        QList<UiProxyWidget *> GetAllProxyWidgets();

        //! Get proxy widget pointer by name
        UiProxyWidget *GetProxyWidget(const QString &widget_name);

        //!
        void SceneRectChanged(const QRectF &new_scene_rect);
        void Connected();
        void Disconnected();

    private:
        void InitMasterLayout();
        void ClearContainerLayout();

        QGraphicsView *ui_view_;
        
        QGraphicsLinearLayout *container_layout_;
        QGraphicsWidget *container_widget_;
        
        CoreUi::MainPanel *main_panel_;
        UiProxyWidget *login_widget_;
        UiProxyWidget *main_panel_proxy_widget_;

        Foundation::Framework *framework_;
    };
}

#endif // incl_UiModule_UiSceneManager_h

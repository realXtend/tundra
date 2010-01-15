// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_UiSceneManager_h
#define incl_UiModule_UiSceneManager_h

#include "UiModuleApi.h"

#include <Foundation.h>

#include <QObject>
#include <QList>

class QRectF;
class QGraphicsLinearLayout;
class QGraphicsWidget;

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
        //! Constructor.
        //! \param framework Framework pointer.
        //! \param ui_view UI view for this scene manager.
        UiSceneManager(Foundation::Framework *framework, QGraphicsView *ui_view);

        //! Destructor.
        ~UiSceneManager();

    public slots:
        //! Adds a Qt Widget to the current scene, returns the added QGraphicsProxyWidget.
        //! The caller of this function is the owner of the proxy widget.
        //! \param widget Widget to be added to the scene.
        UiProxyWidget* AddWidgetToCurrentScene(QWidget *widget);

        //! Adds a Qt Widget to the current scene with Naali widget properties, returns the added QGraphicsProxyWidget
        //! \param widget Widget to be added to the scene.
        //! \param widget_properties Properties for the widget.
        UiProxyWidget* AddWidgetToCurrentScene(QWidget *widget, const UiServices::UiWidgetProperties &widget_properties);

        //! Adds a already created UiProxyWidget into the scene, used for 3D to canvas swaps
        //! \param widget Proxy widget.
        bool AddProxyWidget(UiServices::UiProxyWidget *widget);

        //! Remove a proxy widget from scene if it exist there
        //! Doesn't delete the widget, only removes it from the graphics scene.
        //! \param widget Proxy widget.
        void RemoveProxyWidgetFromCurrentScene(UiProxyWidget *widget);

        //! Get the inworld controls
        CoreUi::MainPanel *GetMainPanel() const { return main_panel_; }

        //! Gets all present proxy widgets from scene
        QList<UiProxyWidget *> GetAllProxyWidgets();

        //! Get proxy widget pointer by name
        //! \return Pointer to the widget, or null if not found.
        UiProxyWidget *GetProxyWidget(const QString &widget_name);

        //!
        void SceneRectChanged(const QRectF &new_scene_rect);

        //!
        void Connect();

        //!
        void Disconnect();

    signals:
        //!
        void Connected();

        //!
        void Disconnected();

    private:
        Q_DISABLE_COPY(UiSceneManager);

        //!
        void InitMasterLayout();

        //!
        void ClearContainerLayout();

        //!
        QGraphicsView *ui_view_;

        //!
        QGraphicsLinearLayout *container_layout_;

        //!
        QGraphicsWidget *container_widget_;

        //! Main panel.
        CoreUi::MainPanel *main_panel_;

        //! Proxy widget for the login ui.
        UiProxyWidget *login_widget_;

        //! Proxy widget for the main panel.
        UiProxyWidget *main_panel_proxy_widget_;

        //! Framework pointer.
        Foundation::Framework *framework_;
    };
}

#endif // incl_UiModule_UiSceneManager_h

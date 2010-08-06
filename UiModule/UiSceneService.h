/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   UiSceneService.h
 *  @brief  
 */

#ifndef incl_UiModule_UiSceneService_h
#define incl_UiModule_UiSceneService_h

#include "UiServiceInterface.h"

namespace UiServices
{
    class UiModule;

    /** Implements UiServiceInterface and provides ...
     *  
     */
    class UiSceneService : public Foundation::UiServiceInterface
    {
        Q_OBJECT

    public:
        /** Constuctor.
         *  @param owner Owner module.
         */
        UiSceneService(UiModule *owner);

        /// Destructor.
        ~UiSceneService();

    public slots:
        /// UiServiceInterface override.
        QGraphicsProxyWidget *AddWidgetToScene(QWidget *widget);

        /// UiServiceInterface override.
        void AddWidgetToScene(QGraphicsProxyWidget *widget);

        /// UiServiceInterface override.
        void RemoveWidgetFromScene(QWidget *widget);

        /// UiServiceInterface override.
        void RemoveWidgetFromScene(QGraphicsProxyWidget *widget);

        /// UiServiceInterface override.
        void ShowWidget(QWidget *widget) const;

        /// UiServiceInterface override.
        void ShowWidget(QGraphicsProxyWidget *widget) const;

        /// UiServiceInterface override.
        void HideWidget(QWidget *widget) const;

        /// UiServiceInterface override.
        void HideWidget(QGraphicsProxyWidget *widget) const;

        /// UiServiceInterface override.
        void BringWidgetToFront(QWidget *widget) const;

        /// UiServiceInterface override.
        void BringWidgetToFront(QGraphicsProxyWidget *widget) const;

        /// UiServiceInterface override.
        const QGraphicsScene *GetScene(const QString &name) const;

        /// UiServiceInterface override.
        void RegisterScene(const QString &name, QGraphicsScene *scene);

        /// UiServiceInterface override.
        bool UnregisterScene(const QString &name);

        /// UiServiceInterface override.
        bool SwitchToScene(const QString &name);

    private:
        /// In-world scene controller.
        UiModule *owner_;
    };
}

#endif

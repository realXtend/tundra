/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   UiSceneService.h
 *  @brief  Implements UiServiceInterface and provides means of adding widgets to the 
 *          
 */

#ifndef incl_UiServiceModule_UiService_h
#define incl_UiServiceModule_UiService_h

#include "UiServiceInterface.h"

class QGraphicsView;

/** Implements UiServiceInterface and provides means of adding widgets to the 
 *  
 */
class UiService : public Foundation::UiServiceInterface
{
    Q_OBJECT

public:
    /** Constuctor.
     *  @param scene The main graphics view.
     */
    explicit UiService(QGraphicsView *view);

    /// Destructor.
    ~UiService();

public slots:
    /// UiServiceInterface override.
    UiProxyWidget *AddWidgetToScene(QWidget *widget, Qt::WindowFlags flags = Qt::Dialog);

    /// UiServiceInterface override.
    void AddWidgetToScene(UiProxyWidget *widget);

    /// UiServiceInterface override.
    void AddWidgetToMenu(QWidget *widget, const QString &name, const QString &menu, const QString &icon);

    /// UiServiceInterface override.
    void AddWidgetToMenu(UiProxyWidget *widget, const QString &name, const QString &menu, const QString &icon);

    /// UiServiceInterface override.
    virtual void AddWidgetToMenu(QWidget *widget);

    /// UiServiceInterface override.
    void RemoveWidgetFromScene(QWidget *widget);

    /// UiServiceInterface override.
    void RemoveWidgetFromScene(QGraphicsProxyWidget *widget);

    /// UiServiceInterface override.
    void ShowWidget(QWidget *widget) const;

    /// UiServiceInterface override.
    void HideWidget(QWidget *widget) const;

    /// UiServiceInterface override.
    void BringWidgetToFront(QWidget *widget) const;

    /// UiServiceInterface override.
    void BringWidgetToFront(QGraphicsProxyWidget *widget) const;

    /// UiServiceInterface override.
    /// Returns the only UI scene we have.
    QGraphicsScene *GetScene(const QString &name) const { return scene_; }

    /// UiServiceInterface override.
    /// Does nothing.
    void RegisterScene(const QString &name, QGraphicsScene *scene) { }

    /// UiServiceInterface override.
    /// Does nothing.
    bool UnregisterScene(const QString &name) { return false; }

    /// UiServiceInterface override.
    /// Does nothing.
    bool SwitchToScene(const QString &name) { return false; }

private:
    //! Pointer to main QGraphicsView
    QGraphicsView *view_;

    //! Pointer to inworld widget scene
    QGraphicsScene *scene_;
};

#endif

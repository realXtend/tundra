// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "SceneWidgetComponentsApi.h"
#include "SceneWidgetComponents.h"
#include "IComponent.h"
#include "Math/float3.h"
#include "Math/float2.h"

#include "OgreModuleFwd.h"
#include "Renderer.h"

#include "InputFwd.h"
#include "AssetFwd.h"
#include "AssetReference.h"

#include <QWidget>
#include <QTimer>
#include <QImage>
#include <QEvent>
#include <QPointer>

class QGraphicsView;
class EC_Billboard;

/// Attaches a billboard with UI widget to an entity.
/** Depends on EC_Billboard*/
class SCENEWIDGET_MODULE_API EC_WidgetBillboard : public IComponent
{
    Q_OBJECT
    COMPONENT_NAME("EC_WidgetBillboard", 42)

    friend class SceneWidgetComponents;

public:
    explicit EC_WidgetBillboard(Scene* scene);
    ~EC_WidgetBillboard();

    /// Asset reference to the UI file where the source widget will be instantiated.
    Q_PROPERTY(AssetReference uiRef READ getuiRef WRITE setuiRef);
    DEFINE_QPROPERTY_ATTRIBUTE(AssetReference, uiRef);

    /// Sets widget 3D scene visibility
    Q_PROPERTY(bool visible READ getvisible WRITE setvisible);
    DEFINE_QPROPERTY_ATTRIBUTE(bool, visible);

    /// Sets if this the target widget accepts input
    Q_PROPERTY(bool acceptInput READ getacceptInput WRITE setacceptInput);
    DEFINE_QPROPERTY_ATTRIBUTE(bool, acceptInput);

    /// 3D widget position in relation to the placeable component
    Q_PROPERTY(float3 position READ getposition WRITE setposition);
    DEFINE_QPROPERTY_ATTRIBUTE(float3, position);

    /// Pixels per world unit (we'll call them meters for simplicity)
    Q_PROPERTY(int ppm READ getppm WRITE setppm);
    DEFINE_QPROPERTY_ATTRIBUTE(int, ppm);

public slots:
    /// Update rendering.
    void Render();

private slots:
    // Internal render called by a delay timer from Render(), due the fact that qt does resize etc operations lazily.
    void RenderInternal();

    /// Returns if the component is prepared.
    bool IsPrepared();

    /// Prepares the component.
    void PrepareComponent();

    /// Monitors removed components.
    void ComponentRemoved(IComponent *component, AttributeChange::Type change);

    /// Returns the unique EC_Billboard for this component (in the parent entity).
    EC_Billboard *GetBillboardComponent();

    /// Handles uiRef asset load.
    void OnUiAssetLoaded(AssetPtr asset);

    /// Handles uiRef asset load fail.
    void OnUiAssetLoadFailed(IAssetTransfer *transfer, QString reason);

    /// Handles main input context mouse events. 
    /// @note This shoul only be called if raycast->component == this.
    void OnMouseEvent(MouseEvent *mEvent, RaycastResult *raycast);

    /// Sends a mouse event to the widget container scene that redirects the input to the actual widget.
    /// @return bool True if event was handled, false otherwise.
    bool SendWidgetMouseEvent(QPoint pos, QEvent::Type type, Qt::MouseButton button, Qt::KeyboardModifier modifier = Qt::NoModifier);

    /// Sends focus out event for the widget.
    bool SendFocusOutEvent();
    
    /// Checks if we have "unacked" mouse press events pending or mouse hover out to be sent out.
    void CheckMouseState();

signals:
    /// Emitted when the 'uiRef' has been loaded successfully. This is a great place to 
    /// connect to the widgets UI signals to your logic eg. pressing buttons.
    /// @param widget QWidget ptr of the instantiated widget.
    void WidgetReady(QWidget *widget);

    /// Emitted when widget receives a mouse event. This enables you to do for example click logic to QWidgets that
    /// do not have proper signals to get click events to scripting eg. QLabel or QFrame.
    /// @param widget QWidget ptr that mouse event happened on.
    /// @param type QEvent::Type enum type of the event.
    /// @param buttom Qt::MouseButton enum of what button was pressed.
    void WidgetMouseEvent(QWidget *widget, QEvent::Type type, Qt::MouseButton button);

    /// Emitted when mouse hovers out of the widget and there for the whole EC_WidgetBillboard rendering area.
    /// As there is no Qt event for "move out" or "hover out" this is a separate signals. When this signal
    /// is sent you can be sure mouse is not on top of any visible widget. For hover in track the WidgetMouseEvent
    /// signal with QEvent::MouseMove.
    void WidgetMouseHoverOut();

protected:
    /// QObject override.
    bool eventFilter(QObject *obj, QEvent *e);

private:
    /// Monitors attribute changes.
    void AttributesChanged();

    void HandlePPMChange();

    // Widget and its container.
    QPointer<QWidget> widget_;
    QGraphicsView *widgetContainer_;

    // Asset related ptrs.
    AssetPtr materialAsset_;
    AssetPtr textureAsset_;
    AssetRefListener *refListener_;

    // Asset related identifiers.
    QString uniqueMaterialName_;
    QString uniqueTextureName_;
    QString billboardCompName_;
    QString cloneMaterialRef_;

    // Rendering related variables.
    QTimer renderTimer_;
    QImage renderBuffer_;

    // Tracking booleans.
    bool rendering_;
    bool leftPressReleased_;
    bool trackingMouseMove_;

    // Renderer ptr.
    OgreRenderer::RendererPtr renderer_;    
};

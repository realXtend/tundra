// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "SceneWidgetComponentsApi.h"
#include "IComponent.h"
#include "SceneFwd.h"
#include "AssetFwd.h"

#include <QTimer>
#include <QString>
#include <QMenu>

class EC_WidgetCanvas;
class EC_Mesh;
class RaycastResult;

namespace Ogre
{
    class TextureUnitState;
}

/// Shows a slideshow of texture on 3D object.
/** Depends on EC_WidgetCanvas and EC_Mesh. */
class SCENEWIDGET_MODULE_API EC_SlideShow : public IComponent
{
    Q_OBJECT
    COMPONENT_NAME("EC_SlideShow", 41)

public:
    /// List of slide asset references (to texture assets).
    Q_PROPERTY(QVariantList slides READ getslides WRITE setslides);
    DEFINE_QPROPERTY_ATTRIBUTE(QVariantList, slides);

    /// Interval in seconds when to change to next slide automatically. Set to 0 to stop automatic slide switching.
    Q_PROPERTY(int slideChangeInterval READ getslideChangeInterval WRITE setslideChangeInterval);
    DEFINE_QPROPERTY_ATTRIBUTE(int, slideChangeInterval);

    /// Current slide index.
    Q_PROPERTY(int currentSlideIndex READ getcurrentSlideIndex WRITE setcurrentSlideIndex);
    DEFINE_QPROPERTY_ATTRIBUTE(int, currentSlideIndex);

    /// Rendering target submesh index.
    Q_PROPERTY(int renderSubmeshIndex READ getrenderSubmeshIndex WRITE setrenderSubmeshIndex);
    DEFINE_QPROPERTY_ATTRIBUTE(int, renderSubmeshIndex);

    /// If rendering to the target submesh index is enabled
    Q_PROPERTY(bool enabled READ getenabled WRITE setenabled);
    DEFINE_QPROPERTY_ATTRIBUTE(bool, enabled);

    /// Boolean for interactive mode, if true it will show context menus on mouse click events.
    Q_PROPERTY(bool interactive READ getinteractive WRITE setinteractive);
    DEFINE_QPROPERTY_ATTRIBUTE(bool, interactive);

    /// Boolean for illuminating the webview. This means the materials emissive will be manipulated to show the webview with full bright always.
    /// If illuminating is true there are no shadows affecting the light, otherwise shadows will be shown.
    Q_PROPERTY(bool illuminating READ getilluminating WRITE setilluminating);
    DEFINE_QPROPERTY_ATTRIBUTE(bool, illuminating);

    /// Constuctor.
    explicit EC_SlideShow(Scene *scene);

    /// Destructor.
    ~EC_SlideShow();

public slots:
    /// Shows slide from index. If index is out of bounds nothing is done.
    void ShowSlide(int index);

    /// Shows next slide, if current slide is the last one shows the first slide.
    void NextSlide();

    /// Shows the previous slide, if current slide is the first one shows the last slide.
    void PreviousSlide();

    /// Goes to the slide show start (index 0).
    void GoToStart();

    /// Goes to the end of the slide show (index len-1).
    void GoToEnd();

    /// Get the context menu. This can be handy if you don't want to have interactive attribute on,
    /// but still want to show the context menu in your own code.
    /// @note The QMenu will destroy itself when closed, you don't need to free the ptr.
    QMenu *GetContextMenu();

private slots:
    /// Handler for window resizes.
    void WindowResized();

    /// Resize timeout to update the rendering.
    void ResizeTimeout();

    /// Prepares everything related to the parent widget and other needed components.
    void PrepareComponent();

    /// One of our listeners is signaling that a texture has been loaded.
    void TextureLoaded(AssetPtr asset);
    
    /// One of our listeners is signaling asset transfer failed.
    void TextureLoadFailed(IAssetTransfer *transfer, QString reason);

    /// Handle AssetAPI forget asset signals.
    void AssetRemoved(AssetPtr asset);

    /// Get render target texture unit state.
    Ogre::TextureUnitState *GetRenderTextureUnit();

    /// Returns if all the internals have been prepared and we are ready for use.
    bool IsPrepared();

    /// Handler when EC_Mesh emits that the mesh is ready.
    void TargetMeshReady();

    /// Handler when EC_Mesh emits that a material has changed on one of its sub meshes.
    /// We inspect if the index is same as we are rendering to. If this is detected we re-apply our material to the sub mesh.
    void TargetMeshMaterialChanged(uint index, const QString &material);

    /// If user select invalid submesh, this function is invoked with a delay and the value is reseted to 0.
    void ResetSubmeshIndex();

    /// Monitors this entitys added components.
    void ComponentAdded(IComponent *component, AttributeChange::Type change);

    /// Monitors this entitys removed components.
    void ComponentRemoved(IComponent *component, AttributeChange::Type change);

    /// Get parent entitys EC_Mesh. Return 0 if not present.
    EC_Mesh *GetMeshComponent();

    /// Get parent entitys (our unique) EC_WidgetCanvas. Return 0 if not present.
    EC_WidgetCanvas *GetSceneCanvasComponent();

    /// Monitors entity mouse clicks.
    void EntityClicked(Entity *entity, Qt::MouseButton button, RaycastResult *raycastResult);

private:
    /// Monitors this components Attribute changes.
    void AttributesChanged();

    /// Timer that changes slides automatically according to slideChangeInterval attribute.
    QTimer changeTimer_;

    /// Timer for window resize event render updates.
    QTimer resizeRenderTimer_;

    /// Unique scene canvas component name for this component to utilize.
    QString sceneCanvasName_;

    /// Asset reference listeners for our presentation slides.
    QList<AssetRefListener*> assetListeners_;

    /// Currently applied texture ref.
    QString currentTextureRef_;

    /// If we are on a server or client.
    bool isServer_;
};

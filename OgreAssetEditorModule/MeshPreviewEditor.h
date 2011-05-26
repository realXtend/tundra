// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreAssetEditorModule_MeshPreviewEditor_h
#define incl_OgreAssetEditorModule_MeshPreviewEditor_h

#include "InputAPI.h"
#include "MouseEvent.h"
#include "AssetFwd.h"
#include "OgreModuleFwd.h"
#include "OgreAssetEditorModuleApi.h"

#include <QWidget>
#include <QLabel>
#include <QImage>

class QPushButton;

/// Label is used to display the mesh in image format.
class MeshPreviewLabel: public QLabel
{
    Q_OBJECT

public:
    MeshPreviewLabel(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    virtual ~MeshPreviewLabel();

signals:
    void sendMouseEvent(QMouseEvent *e);
    void sendWheelEvent(QWheelEvent* e);

protected:
     void mouseMoveEvent(QMouseEvent *e);
     void mousePressEvent(QMouseEvent* e);
     void mouseReleaseEvent(QMouseEvent* e);
     void wheelEvent(QWheelEvent* e);
};

/// MeshPrevieEditor is used to view meshes
class ASSET_EDITOR_MODULE_API MeshPreviewEditor: public QWidget
{
    Q_OBJECT

public:
    MeshPreviewEditor(Framework *framework, QWidget* parent = 0);

    virtual ~MeshPreviewEditor();

    void RequestMeshAsset(const QString &asset_id);
    QImage ConvertToQImage(const u8 *raw_image_data, uint width, uint height, uint channels);
    void Open(const QString& asset_id);

    static MeshPreviewEditor *OpenMeshPreviewEditor(Framework *framework, const QString &asset_id, QWidget* parent = 0);

public slots:
    void Update();
    void MouseEvent(QMouseEvent* event);
    void MouseWheelEvent(QWheelEvent* ev);

signals:
    /// This signal is emitted when the editor is closed.
    void Closed(const QString &inventory_id);

private slots:
    /// Delete this object.
    void Deleted() { delete this; }

private:
    void InitializeEditorWidget();
    void CreateRenderTexture();
    void AdjustScene();

    Framework *framework_;

    QWidget *mainWidget_;
    QPushButton *okButton_;
    QString assetId_;

    QPointF lastPos_;
    int camAlphaAngle_;
    QString mesh_id_;
    // Mid button roll.
    double mouseDelta_;
    InputContextPtr meshInputContext_;
    MeshPreviewLabel* label_;

    // For mesh viewing
    OgreRenderer::RendererPtr renderer_;
    Ogre::SceneManager* manager_;
    Ogre::Camera* camera_;
    Ogre::Entity* entity_;
    Ogre::SceneNode* scene_;
    Ogre::SceneNode* root_scene_;
    Ogre::Light* newLight_;
    Ogre::RenderTexture* render_texture_;
    int width_;
    int height_;
};

#endif

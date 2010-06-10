#include "StableHeaders.h"
#include "Framework.h"
#include "DebugOperatorNew.h"
#include "MeshPreviewEditor.h"
#include "OgreAssetEditorModule.h"

#include <UiModule.h>
#include "Inworld/View/UiProxyWidget.h"
#include "Inworld/View/UiWidgetProperties.h"
#include "Inworld/InworldSceneController.h"
#include "ModuleManager.h"
#include "AssetInterface.h"

#include <QUiLoader>
#include <QFile>
#include <QLayout>
#include <QPushButton>

#include "MemoryLeakCheck.h"

namespace Naali
{
    MeshPreviewLabel::MeshPreviewLabel(QWidget *parent, Qt::WindowFlags flags):
        QLabel(parent, flags)
    {

    }

    MeshPreviewLabel::~MeshPreviewLabel()
    {

    }

    MeshPreviewEditor::MeshPreviewEditor(Foundation::Framework *framework,
                                           const QString &inventory_id,
                                           const asset_type_t &asset_type,
                                           const QString &name,
                                           QWidget *parent):
        QWidget(parent),
        framework_(framework),
        assetType_(asset_type),
        inventoryId_(inventory_id),
        okButton_(0)
    {
        setObjectName(name);
        InitializeEditorWidget();
    }

    MeshPreviewEditor::~MeshPreviewEditor()
    {
    
    }

    void MeshPreviewEditor::HandleAssetReady(Foundation::AssetPtr asset)
    {
        /*
        boost::shared_ptr<OgreRenderer::OgreRenderingModule> rendering_module = 
            framework_->GetModuleManager()->GetModule<OgreRenderer::OgreRenderingModule>(Foundation::Module::MT_Renderer).lock();
        
        if (!rendering_module)
            return;

        OgreRenderer::RendererPtr renderer = rendering_module->GetRenderer();
        
        // Create scenenode and attach camera to it
        
        Ogre::Camera *screenshot_cam = GetSceneManager()->createCamera("ScreenshotCamera");
        Ogre::SceneNode *cam_node = renderer->GetSceneManager()->createSceneNode("MeshShotNode");
        cam_node->attachObject(screenshot_cam);

         // Render camera view to texture and save to file
        Ogre::TexturePtr avatar_screenshot = Ogre::TextureManager::getSingleton().createManual(
            "ScreenshotTexture", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
            Ogre::TEX_TYPE_2D, window_width, window_height, 0, Ogre::PF_A8R8G8B8, Ogre::TU_RENDERTARGET);

        Ogre::RenderTexture *render_texture = avatar_screenshot->getBuffer()->getRenderTarget();
        Ogre::Viewport *vp = render_texture->addViewport(screenshot_cam);
        render_texture->update();
       */

        qDebug()<<" Asset ready?";       
        //TODO! Add asset ready code in here.

    }

    void MeshPreviewEditor::Closed()
    {
        boost::shared_ptr<UiServices::UiModule> ui_module =
            framework_->GetModuleManager()->GetModule<UiServices::UiModule>(Foundation::Module::MT_UiServices).lock();
        if (!ui_module.get())
            return;

        ui_module->GetInworldSceneController()->RemoveProxyWidgetFromScene(this);

        emit Closed(inventoryId_, assetType_);
    }

    void MeshPreviewEditor::InitializeEditorWidget()
    {
        // Get QtModule and create canvas
        boost::shared_ptr<UiServices::UiModule> ui_module = 
            framework_->GetModuleManager()->GetModule<UiServices::UiModule>(Foundation::Module::MT_UiServices).lock();
        if (!ui_module.get())
            return;

        // Create widget from ui file
        QUiLoader loader;
        QFile file("./data/ui/mesh_preview.ui");
        if (!file.exists())
        {
            OgreAssetEditorModule::LogError("Cannot find OGRE Script Editor .ui file.");
            return;
        }
        mainWidget_ = loader.load(&file);
        file.close();

        QVBoxLayout *layout = new QVBoxLayout;
        setLayout(layout);
        layout->addWidget(mainWidget_);
        layout->setContentsMargins(0, 0, 0, 0);

        okButton_ = mainWidget_->findChild<QPushButton *>("okButton");
        QObject::connect(okButton_, SIGNAL(clicked()), this, SLOT(Closed()));

        // Add widget to UI via ui services module
        UiServices::UiProxyWidget *proxy = ui_module->GetInworldSceneController()->AddWidgetToScene(this, UiServices::UiWidgetProperties(tr("Mesh: ") + objectName(), UiServices::SceneWidget));
        QObject::connect(proxy, SIGNAL(Closed()), this, SLOT(Closed()));
        proxy->show();
        ui_module->GetInworldSceneController()->BringProxyToFront(proxy);
    }
}
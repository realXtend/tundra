// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "Framework.h"
#include "DebugOperatorNew.h"
#include "MeshPreviewEditor.h"
#include "OgreAssetEditorModule.h"
#include "OgreRenderingModule.h"
#include "OgreMeshResource.h"
#include <OgreLight.h>

#include <UiModule.h>
#include "Inworld/View/UiProxyWidget.h"
#include "Inworld/View/UiWidgetProperties.h"
#include "Inworld/InworldSceneController.h"
#include "ModuleManager.h"
#include "AssetInterface.h"
#include "ResourceInterface.h"


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
                                           const QString &assetID,
                                           QWidget *parent):
        QWidget(parent),
        framework_(framework),
        assetType_(asset_type),
        inventoryId_(inventory_id),
        okButton_(0),
        assetId_(assetID),
        request_tag_(0)
    {
        setObjectName(name);
        InitializeEditorWidget();
        RequestMeshAsset(assetId_);
    }

    MeshPreviewEditor::~MeshPreviewEditor()
    {
    
    }

   
    void MeshPreviewEditor::HandleResouceReady(Resource::Events::ResourceReady *res)
    {
        if(request_tag_ == res->tag_)
        {
            //Foundation::TextureInterface *tex = dynamic_cast<Foundation::TextureInterface *>(res->resource_.get());

            boost::shared_ptr<OgreRenderer::OgreRenderingModule> rendering_module = 
                framework_->GetModuleManager()->GetModule<OgreRenderer::OgreRenderingModule>().lock();

            if (!rendering_module)
                return;

             OgreRenderer::RendererPtr renderer = rendering_module->GetRenderer();
        
             // Hide ui
             renderer->HideCurrentWorldView();
             
             // Create scenenode and attach camera to it
        
             int iWidth = 400;
             int iHeight = 400;
             OgreRenderer::OgreRootPtr root = renderer->GetRoot();
             Ogre::SceneManager* manager = root->createSceneManager(Ogre::ST_GENERIC,"MeshShotManager");
             
           
             Ogre::Camera* camera = manager->createCamera("MeshShotCamera");
             
             Ogre::Entity* entity = manager->createEntity("entity", res->id_);
             entity->setMaterialName("BaseWhite");
             
             Ogre::AxisAlignedBox box = entity->getBoundingBox();
             
             Ogre::Vector3 boxCenterPos = box.getHalfSize();
             Ogre::Vector3 scenePos(0.0,-boxCenterPos.y,0.0);
             Ogre::Vector3 volume= box.getSize();
             
             // Get biggest side :
             double biggest = volume.x;
             if ( biggest < volume.y )
                 biggest = volume.y;
             if ( biggest < volume.z )
                 biggest = volume.z;


             Ogre::SceneNode* scene = manager->createSceneNode("MeshShotNode");
             scene->attachObject(entity);
             scene->showBoundingBox(true);
             //scene->setPosition(scenePos);

             Ogre::SceneNode* root_scene = manager->getRootSceneNode();
             root_scene->addChild(scene);
            
             //scene->attachObject(camera);

            
             camera->setNearClipDistance(0.1f);
             camera->setFarClipDistance(2000.f);
             Ogre::Vector3 pos(0,0,biggest * 2);
             //camera->setPosition(scene->getPosition().x, scene->getPosition().y, scene->getPosition().z - biggest * 2);
             camera->setPosition(pos);
             camera->lookAt(Ogre::Vector3(0,0,0));
             //camera->lookAt(scene->getPosition());

             Ogre::Light* newLight = manager->createLight("light");
             //newLight->setPosition(pos);
             newLight->setDirection(Ogre::Vector3(0,0,-1));
             newLight->setType(Ogre::Light::LT_DIRECTIONAL);
             newLight->setDiffuseColour(Ogre::ColourValue::White);
             manager->setAmbientLight(Ogre::ColourValue::Black);
            

             // Render camera view to texture 
          
            Ogre::TexturePtr tex = Ogre::TextureManager::getSingleton().createManual(
                "MeshShotTexture", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                Ogre::TEX_TYPE_2D, iWidth, iHeight, 0, Ogre::PF_A8R8G8B8, Ogre::TU_RENDERTARGET);

            Ogre::RenderTexture* render_texture = tex->getBuffer()->getRenderTarget();
            Ogre::Viewport* vieport = render_texture->addViewport(camera);
            //vieport->setBackgroundColour(Ogre::ColourValue(0.5,0.5,0.5,0.5));
            render_texture->update();
           
            Ogre::Box bounds(0, 0, iWidth, iHeight);
            Ogre::uchar* pixelData = new Ogre::uchar[iWidth * iHeight * 4];
            Ogre::PixelBox pixels(bounds, Ogre::PF_A8R8G8B8, pixelData);
            
            render_texture->copyContentsToMemory(pixels, Ogre::RenderTarget::FB_AUTO);

            // Create image
            u8* p = static_cast<u8 *>(pixels.data);
            int width = pixels.getWidth();
            int height = pixels.getHeight();
            
            QImage img = ConvertToQImage(p, width, height, 4);

             if(!img.isNull())
                    {
                        // Take image's original size.
                        //imageSize_ = QSize(tex->GetWidth(), tex->GetHeight());
                        //if(headerLabel_)
                        //    headerLabel_->setText(headerLabel_->text() + QString(" %1 x %2").arg(tex->GetWidth()).arg(tex->GetHeight()));

                     
                            QLabel* imageLabel = mainWidget_->findChild<QLabel *>("meshImageLabel");

                            imageLabel->setPixmap(QPixmap::fromImage(img));
                            //UseTextureOriginalSize(true);
                            //scaleLabel_->setText("Image scale: 1:1");
                        
                    }
              
            boost::shared_ptr<UiServices::UiModule> ui_module = 
            framework_->GetModuleManager()->GetModule<UiServices::UiModule>().lock();
            
            if (!ui_module.get())
                return;

            proxy_->show();
            ui_module->GetInworldSceneController()->BringProxyToFront(proxy_);

            Ogre::TextureManager::getSingleton().remove("MeshShotTexture");
            root_scene->removeAllChildren();
            manager->destroySceneNode(scene);
            manager->destroyCamera(camera);
            manager->destroyLight(newLight);
            root->destroySceneManager(manager);

            delete[] pixelData;

            renderer->ShowCurrentWorldView();
        }

    }

    void MeshPreviewEditor::RequestMeshAsset(const QString &asset_id)
    {
         boost::shared_ptr<OgreRenderer::OgreRenderingModule> rendering_module = 
            framework_->GetModuleManager()->GetModule<OgreRenderer::OgreRenderingModule>().lock();
        
        OgreRenderer::RendererPtr renderer = rendering_module->GetRenderer();
        
            if (renderer != 0)
            {
                request_tag_ = renderer->RequestResource(asset_id.toStdString(), OgreRenderer::OgreMeshResource::GetTypeStatic());

            
            }
        

    }


   

    void MeshPreviewEditor::Closed()
    {
        boost::shared_ptr<UiServices::UiModule> ui_module =
            framework_->GetModuleManager()->GetModule<UiServices::UiModule>().lock();
        if (!ui_module.get())
            return;

        ui_module->GetInworldSceneController()->RemoveProxyWidgetFromScene(this);

        emit Closed(inventoryId_, assetType_);
    }

    void MeshPreviewEditor::InitializeEditorWidget()
    {
        // Get QtModule and create canvas
        boost::shared_ptr<UiServices::UiModule> ui_module = 
            framework_->GetModuleManager()->GetModule<UiServices::UiModule>().lock();
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
        proxy_ = ui_module->GetInworldSceneController()->AddWidgetToScene(this, UiServices::UiWidgetProperties(tr("Mesh: ") + objectName(), UiServices::SceneWidget));
        QObject::connect(proxy_, SIGNAL(Closed()), this, SLOT(Closed()));
        //proxy->show();
        //ui_module->GetInworldSceneController()->BringProxyToFront(proxy_);
    }


    QImage MeshPreviewEditor::ConvertToQImage(const u8 *raw_image_data, int width, int height, int channels)
    {
        uint img_width_step = width * channels; 
        QImage image;
        if(width <= 0 && height <= 0 && channels <= 0)
            return image;

        if(channels == 3)// For RGB32
        {
            image = QImage(QSize(width, height), QImage::Format_RGB888);
            for(uint h = 0; h < height; h++)
            {
                for(uint w = 0; w < width; w++)
                {
                    u8 color[3];
                    for(uint comp = 0; comp < 3; comp++)
                    {
                        uint index = (h % height) * (img_width_step) + ((w * channels) % (img_width_step)) + comp;
                        color[comp] = raw_image_data[index];
                    }
                    image.setPixel(w, h, qRgb(color[0], color[1], color[2]));
                }
            }
        }
        else if(channels == 4)// For ARGB32
        {
            image = QImage(QSize(width, height), QImage::Format_ARGB32);
            for(uint h = 0; h < height; h++)
            {
                for(uint w = 0; w < width; w++)
                {
                    u8 color[4];
                    for(uint comp = 0; comp < 4; comp++)
                    {
                        uint index = (h % height) * (img_width_step) + ((w * channels) % (img_width_step)) + comp;
                        color[comp] = raw_image_data[index];
                    }
                    image.setPixel(w, h, qRgba(color[0], color[1], color[2], color[3]));
                }
            }
        }
        else if(channels == 1)// For GrayScale
        {
            image = QImage(QSize(width, height), QImage::Format_RGB888);
            for(uint h = 0; h < height; h++)
            {
                for(uint w = 0; w < width; w++)
                {
                    u8 color[1];
                    for(uint comp = 0; comp < 1; comp++)
                    {
                        uint index = (h % height) * (img_width_step) + ((w * channels) % (img_width_step)) + comp;
                        color[comp] = raw_image_data[index];
                    }
                    image.setPixel(w, h, qRgb(color[0], color[0], color[0]));
                }
            }
        }
        else if(channels == 2) // contains Grayscale and Alfa channels.
        {
            // TODO! 2 channel image contain grayscale and alfa channels this code wasn't tested and might not work as planned.
            // This image format is not so much used so it might be a good idea to remove this if necessary.
            image = QImage(QSize(width, height), QImage::Format_ARGB32);
            for(uint h = 0; h < height; h++)
            {
                for(uint w = 0; w < width; w++)
                {
                    u8 color[2];
                    for(uint comp = 0; comp < 2; comp++)
                    {
                        uint index = (h % height) * (img_width_step) + ((w * channels) % (img_width_step)) + comp;
                        color[comp] = raw_image_data[index];
                    }
                    image.setPixel(w, h, qRgba(color[0], color[0], color[0], color[1]));
                }
            }
        }

        return image;
    }

}
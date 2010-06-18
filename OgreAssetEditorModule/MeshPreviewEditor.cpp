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
#include "CoreMath.h"


#include <QUiLoader>
#include <QFile>
#include <QLayout>
#include <QPushButton>

#include "MemoryLeakCheck.h"

namespace Naali
{
    MeshPreviewLabel::MeshPreviewLabel(QWidget *parent, Qt::WindowFlags flags):
        QLabel(parent, flags), leftPressed_(false), rightPressed_(false)
    {
         setMouseTracking(true);
    }

    MeshPreviewLabel::~MeshPreviewLabel()
    {

    }

    void MeshPreviewLabel::mousePressEvent(QMouseEvent* ev)
    {
        if ( ev->button() == Qt::LeftButton ) 
            leftPressed_ = true;
        if ( ev->button() == Qt::RightButton )
            rightPressed_  = true;
    }

    void MeshPreviewLabel::mouseReleaseEvent(QMouseEvent* ev)
    {
        if ( ev->button() == Qt::LeftButton ) 
            leftPressed_ = false;
        if ( ev->button() == Qt::RightButton )
            rightPressed_  = false;
    }

    void MeshPreviewLabel::wheelEvent(QWheelEvent* ev)
    {
        sendWheelEvent(ev);
    }

    void MeshPreviewLabel::mouseMoveEvent(QMouseEvent *event)
    {
        if ( leftPressed_ && ( rightPressed_ || event->buttons() == Qt::RightButton) )
        {
            QMouseEvent ev(QEvent::MouseMove,event->pos(),Qt::NoButton, Qt::LeftButton | Qt::RightButton, Qt::NoModifier);
            sendMouseEvent(&ev, true);      
            return;
        }
        
        if ( rightPressed_ || event->buttons() == Qt::RightButton )
        {
            QMouseEvent ev(QEvent::MouseMove,event->pos(),Qt::RightButton, Qt::RightButton, Qt::NoModifier );
            sendMouseEvent(&ev, false);      
            return;
        }

       
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
        request_tag_(0),
        lastPos_(QPointF()),
        camAlphaAngle_(0),
        mesh_id_(""),
        mouseDelta_(0)
      
    {
        setObjectName(name);
        InitializeEditorWidget();
        RequestMeshAsset(assetId_);
        setMouseTracking(true);
    }

    MeshPreviewEditor::~MeshPreviewEditor()
    {}

   
    void MeshPreviewEditor::HandleResouceReady(Resource::Events::ResourceReady *res)
    {
        if(request_tag_ == res->tag_)
        {
           mesh_id_ =  res->id_.c_str();
           Update();
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

    void MeshPreviewEditor::Update()
    {
            if ( mesh_id_ == "" )
                return;

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
             
             Ogre::Entity* entity = manager->createEntity("entity", mesh_id_.toStdString());
             entity->setMaterialName("BaseWhite");
             
             Ogre::AxisAlignedBox box = entity->getBoundingBox();
             Ogre::Vector3 boxCenterPos = box.getHalfSize();
            
             
             Ogre::Vector3 volume= box.getSize();
             
             // Get biggest side :
            
             double biggest = boxCenterPos.x;
             if ( biggest < boxCenterPos.y )
                 biggest = boxCenterPos.y;
             if ( biggest < boxCenterPos.z )
                 biggest = boxCenterPos.z;


             Ogre::SceneNode* scene = manager->createSceneNode("MeshShotNode");
             scene->attachObject(entity);
            
             scene->showBoundingBox(true);
             scene->rotate(Ogre::Vector3(0,1,0),Ogre::Radian(camAlphaAngle_ * PI/180.0));            
       

             Ogre::SceneNode* root_scene = manager->getRootSceneNode();
             root_scene->addChild(scene);
           
             camera->setNearClipDistance(0.1f);
             camera->setFarClipDistance(2000.f);
             
             // Camera position vector
             double r = sqrt(2.0) * 2*biggest * tan(camera->getFOVy().valueRadians() * camera->getAspectRatio());
            
             // Adjust r by using mouse roll

             int numDegrees = mouseDelta_ / 8;
             int numSteps = numDegrees / 15;
             r = r + numSteps;

             double x = 0;
             double y = boxCenterPos.y;
             double z = r;

             Ogre::Vector3 pos(x,y,z);
             camera->setPosition(pos);
             camera->lookAt(boxCenterPos);
          
             Ogre::Light* newLight = manager->createLight("light");
       
             newLight->setDirection(Ogre::Vector3(-x,-y,-z));
             newLight->setType(Ogre::Light::LT_DIRECTIONAL);
             newLight->setDiffuseColour(Ogre::ColourValue::White);
             manager->setAmbientLight(Ogre::ColourValue::Black);
            
             // Render camera view to texture 
          
            Ogre::TexturePtr tex = Ogre::TextureManager::getSingleton().createManual(
                "MeshShotTexture", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                Ogre::TEX_TYPE_2D, iWidth, iHeight, 0, Ogre::PF_A8R8G8B8, Ogre::TU_RENDERTARGET);

            Ogre::RenderTexture* render_texture = tex->getBuffer()->getRenderTarget();
            Ogre::Viewport* vieport = render_texture->addViewport(camera);
         
            render_texture->update();
           
            Ogre::Box bounds(0, 0, iWidth, iHeight);
            Ogre::uchar* pixelData = new Ogre::uchar[iWidth * iHeight * 4];
            Ogre::PixelBox pixels(bounds, Ogre::PF_A8R8G8B8, pixelData);
            
            render_texture->copyContentsToMemory(pixels, Ogre::RenderTarget::FB_AUTO);

            // Create image of texture, and show it into label.

            u8* p = static_cast<u8 *>(pixels.data);
            int width = pixels.getWidth();
            int height = pixels.getHeight();
            
            QImage img = ConvertToQImage(p, width, height, 4);

            if(!img.isNull())
            {
                QLabel* imageLabel = mainWidget_->findChild<QLabel *>("meshImageLabel");
                imageLabel->setPixmap(QPixmap::fromImage(img));
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

        okButton_ = mainWidget_->findChild<QPushButton *>("okButton");
        QObject::connect(okButton_, SIGNAL(clicked()), this, SLOT(Closed()));
        
        QVBoxLayout* vLayout = mainWidget_->findChild<QVBoxLayout* >("imageLayout");
        
        MeshPreviewLabel* label = new MeshPreviewLabel(mainWidget_);
        label->setObjectName("meshImageLabel");
        
        label->setMinimumSize(300,300);
    
        // Kind of hack, in this way we assure that we will get mouseMove-event with left and right button pressed.
        QObject::connect(label, SIGNAL(sendMouseEvent(QMouseEvent*,bool)), this, SLOT(MouseEvent(QMouseEvent*,bool)));
        QObject::connect(label, SIGNAL(sendWheelEvent(QWheelEvent*)), this, SLOT(MouseWheelEvent(QWheelEvent*)));
        vLayout->addWidget(label);

        QVBoxLayout *layout = new QVBoxLayout;
        setLayout(layout);
        layout->addWidget(mainWidget_);
        layout->setContentsMargins(0, 0, 0, 0);
  

        // Add widget to UI via ui services module
        proxy_ = ui_module->GetInworldSceneController()->AddWidgetToScene(this, UiServices::UiWidgetProperties(tr("Mesh: ") + objectName(), UiServices::SceneWidget));
        QObject::connect(proxy_, SIGNAL(Closed()), this, SLOT(Closed()));
       
    }

    void MeshPreviewEditor::MouseWheelEvent(QWheelEvent* ev)
    {
        int delta = ev->delta();
        mouseDelta_ += delta;
        Update();
        
    }


    void MeshPreviewEditor::MouseEvent(QMouseEvent* event, bool both)
    {
      
        QPointF pos = event->posF();

        if ( event->buttons() == Qt::RightButton )
        {
            
            if ( pos.x() > lastPos_.x() )
            {
                
                camAlphaAngle_ = camAlphaAngle_ + 5;
                
                if ( camAlphaAngle_ < 0 )
                    camAlphaAngle_ = - abs(camAlphaAngle_) % 360; 
                else
                    camAlphaAngle_ = camAlphaAngle_ % 360;
                
                
                Update();
            }
            else if ( pos.x() < lastPos_.x())
            {
            
                camAlphaAngle_ = camAlphaAngle_ - 5; 
                if ( camAlphaAngle_ < 0 )
                    camAlphaAngle_ = - abs(camAlphaAngle_) % 360; 
                else
                    camAlphaAngle_ = camAlphaAngle_ % 360;
                
                Update();
            }
        
        }
        

       lastPos_ = pos;    
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
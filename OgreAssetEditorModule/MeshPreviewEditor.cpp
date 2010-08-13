// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "MeshPreviewEditor.h"
#include "OgreAssetEditorModule.h"

#include "Renderer.h"
#include "UiModule.h"
#include "UiProxyWidget.h"
#include "UiWidgetProperties.h"
#include "Inworld/InworldSceneController.h"
#include "ModuleManager.h"
#include "AssetInterface.h"
#include "ResourceInterface.h"
#include "CoreMath.h"
#include "OgreRenderingModule.h"

#include <QUiLoader>
#include <QFile>
#include <QLayout>
#include <QPushButton>
#include <QDebug>

#include "MemoryLeakCheck.h"

namespace Naali
{
    MeshPreviewLabel::MeshPreviewLabel(QWidget *parent, Qt::WindowFlags flags):
        QLabel(parent, flags)
    {
         setMouseTracking(true);
    }

    MeshPreviewLabel::~MeshPreviewLabel()
    {}

    void MeshPreviewLabel::mousePressEvent(QMouseEvent* ev)
    {
        if ( ev->buttons() == Qt::RightButton )
            sendMouseEvent(ev);
    }

    void MeshPreviewLabel::mouseReleaseEvent(QMouseEvent* ev)
    {
     //   if ( ev->buttons() == Qt::RightButton )
            sendMouseEvent(ev);
    }

    void MeshPreviewLabel::wheelEvent(QWheelEvent* ev)
    {
        sendWheelEvent(ev);
    }

    void MeshPreviewLabel::mouseMoveEvent(QMouseEvent *event)
    {

        if ( event->buttons() == Qt::RightButton )
            sendMouseEvent(event);
      
       
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
        mouseDelta_(0),
        label_(0),
        manager_(0),
        camera_(0),
        entity_(0),
        scene_(0),
        root_scene_(0),
        newLight_(0),
        render_texture_(0),
        width_(400),
        height_(400)
      
    {
       
        setObjectName(name);
        InitializeEditorWidget();
        RequestMeshAsset(assetId_);
        setMouseTracking(true);
    }

    MeshPreviewEditor::~MeshPreviewEditor()
    {
        // Does not own
        label_ = 0;

        if ( renderer_ != 0)
        {
            OgreRenderer::OgreRootPtr root = renderer_->GetRoot();
            render_texture_->removeAllViewports();

            Ogre::TextureManager::getSingleton().remove(mesh_id_.toStdString().c_str());
            root_scene_->removeAllChildren();
            manager_->destroySceneNode(scene_);
            manager_->destroyCamera(camera_);
            manager_->destroyLight(newLight_);
            
            root->destroySceneManager(manager_);
        }
    }

   
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
            request_tag_ = renderer->RequestResource(asset_id.toStdString(), OgreRenderer::OgreMeshResource::GetTypeStatic());
    }

    void MeshPreviewEditor::Update()
    {
        if ( mesh_id_ == "" )
            return;

        if ( render_texture_ == 0 )
        {
            CreateRenderTexture();
            
        }
        // Hide ui
        renderer_->HideCurrentWorldView();

        AdjustScene();

        render_texture_->update();
           
        Ogre::Box bounds(0, 0, width_, height_);
        Ogre::uchar* pixelData = new Ogre::uchar[width_ * height_ * 4];
        Ogre::PixelBox pixels(bounds, Ogre::PF_A8R8G8B8, pixelData);
        
        render_texture_->copyContentsToMemory(pixels, Ogre::RenderTarget::FB_AUTO);

        // Create image of texture, and show it into label.

        u8* p = static_cast<u8 *>(pixels.data);
        int width = pixels.getWidth();
        int height = pixels.getHeight();
        
        QImage img = ConvertToQImage(p, width, height, 4);

        if(!img.isNull() && label_ != 0)
            label_->setPixmap(QPixmap::fromImage(img));
        
    
        boost::shared_ptr<UiServices::UiModule> ui_module = 
        framework_->GetModuleManager()->GetModule<UiServices::UiModule>().lock();
        
        if (!ui_module.get())
            return;

        proxy_->show();
        ui_module->GetInworldSceneController()->BringProxyToFront(proxy_);

        delete[] pixelData;
        
        // Show ui
        renderer_->ShowCurrentWorldView();  
    }

    void MeshPreviewEditor::AdjustScene()
    {
        if ( scene_ == 0)
            return;
        
        scene_->rotate(Ogre::Vector3(0,1,0),Ogre::Radian(camAlphaAngle_ * PI/180.0));         

        Ogre::AxisAlignedBox box = entity_->getBoundingBox();
        Ogre::Vector3 boxCenterPos = box.getHalfSize();
            
        Ogre::Vector3 volume= box.getSize();
             
        // Get biggest side :
            
        double biggest = boxCenterPos.x;
        if ( biggest < boxCenterPos.y )
            biggest = boxCenterPos.y;
        if ( biggest < boxCenterPos.z )
            biggest = boxCenterPos.z;

        // Create camera position vector
        
        double r = sqrt(2.0) * 2*biggest * tan(camera_->getFOVy().valueRadians() * camera_->getAspectRatio());
    
        // Adjust r by using mouse roll

        int numSteps = ( mouseDelta_ / 8 ) / 15 ;
        r = r + numSteps;

        double x = 0;
        double y = boxCenterPos.y;
        double z = r;

        Ogre::Vector3 pos(x,y,z);

        camera_->setPosition(pos);
        camera_->lookAt(boxCenterPos);

    }

   
    void MeshPreviewEditor::CreateRenderTexture()
    {
          boost::shared_ptr<OgreRenderer::OgreRenderingModule> rendering_module = 
                framework_->GetModuleManager()->GetModule<OgreRenderer::OgreRenderingModule>().lock();

            if (!rendering_module)
                return;

             renderer_ = rendering_module->GetRenderer();
           
             // Create scenenode and attach camera to it
        
             OgreRenderer::OgreRootPtr root = renderer_->GetRoot();
             manager_ = root->createSceneManager(Ogre::ST_GENERIC,mesh_id_.toStdString().c_str());
           
             camera_ = manager_->createCamera(mesh_id_.toStdString().c_str());
             
             entity_ = manager_->createEntity("entity", mesh_id_.toStdString());
             entity_->setMaterialName("BaseWhite");
             
             Ogre::AxisAlignedBox box = entity_->getBoundingBox();
             Ogre::Vector3 boxCenterPos = box.getHalfSize();
            
             Ogre::Vector3 volume= box.getSize();
             
             // Get biggest side :
            
             double biggest = boxCenterPos.x;
             if ( biggest < boxCenterPos.y )
                 biggest = boxCenterPos.y;
             if ( biggest < boxCenterPos.z )
                 biggest = boxCenterPos.z;


             scene_ = manager_->createSceneNode(mesh_id_.toStdString().c_str());      
             scene_->attachObject(entity_);
             scene_->showBoundingBox(true);
             scene_->rotate(Ogre::Vector3(0,1,0),Ogre::Radian(camAlphaAngle_ * PI/180.0));            
       
             root_scene_ = manager_->getRootSceneNode();
             root_scene_->addChild(scene_);
           
             camera_->setNearClipDistance(0.1f);
             camera_->setFarClipDistance(2000.f);
             
             // Create camera position vector
             
             double r = sqrt(2.0) * 2*biggest * tan(camera_->getFOVy().valueRadians() * camera_->getAspectRatio());
            
             // Adjust r by using mouse roll

             int numDegrees = mouseDelta_ / 8;
             int numSteps = numDegrees / 15;
             r = r + numSteps;

             double x = 0;
             double y = boxCenterPos.y;
             double z = r;

             Ogre::Vector3 pos(x,y,z);
             camera_->setPosition(pos);
             camera_->lookAt(boxCenterPos);
          
             newLight_ = manager_->createLight("light");
            
          

             newLight_->setDirection(Ogre::Vector3(-x,-y,-z));
             newLight_->setType(Ogre::Light::LT_DIRECTIONAL);
             newLight_->setDiffuseColour(Ogre::ColourValue::White);
             manager_->setAmbientLight(Ogre::ColourValue::Black);
            
             // Render camera view to texture 
          
            Ogre::TexturePtr tex = Ogre::TextureManager::getSingleton().createManual(
                mesh_id_.toStdString().c_str(), Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                Ogre::TEX_TYPE_2D, width_, height_, 0, Ogre::PF_A8R8G8B8, Ogre::TU_RENDERTARGET);

            render_texture_ = tex->getBuffer()->getRenderTarget();
            render_texture_->setAutoUpdated(false);
            
            Ogre::Viewport* vieport = render_texture_->addViewport(camera_);
         
          

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
        
        label_ = new MeshPreviewLabel(mainWidget_);
        label_->setObjectName("meshImageLabel");
        
        label_->setMinimumSize(300,300);
    

        // Kind of hack, in this way we assure that we will get mouseMove-event with left and right button pressed.
        QObject::connect(label_, SIGNAL(sendMouseEvent(QMouseEvent*)), this, SLOT(MouseEvent(QMouseEvent*)));
        QObject::connect(label_, SIGNAL(sendWheelEvent(QWheelEvent*)), this, SLOT(MouseWheelEvent(QWheelEvent*)));
        vLayout->addWidget(label_);

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

   
    void MeshPreviewEditor::MouseEvent(QMouseEvent* event)
    {
      
        QPointF pos = event->posF();

        if ( event->type() == QEvent::MouseButtonRelease )
        {
            lastPos_ = QPointF(-1.0,-1.0);
            return;
        }

        // After release new mouse move with RMB pressed down comes then we will first time set position same.

        if ( lastPos_ == QPointF(-1.0, -1.0) || event->type() == QEvent::MouseButtonPress)
        {
            lastPos_ = pos;
            return;
        }

        if ( event->buttons() == Qt::RightButton )
        {
            
            if ( pos.x() > lastPos_.x() )
            {
                int width = label_->size().width();
                double pix = (2*360.0) / width;
                
                camAlphaAngle_ = camAlphaAngle_ + pix * abs(pos.x() - lastPos_.x());
                
                if ( camAlphaAngle_ < 0 )
                    camAlphaAngle_ = - ( abs(camAlphaAngle_) % 360 ); 
                else
                    camAlphaAngle_ = camAlphaAngle_ % 360;
             
                
                Update();
            }
            else if ( pos.x() < lastPos_.x())
            {
             
                int width = label_->size().width();
                double pix = (2* 360.0 ) / width;
             
                camAlphaAngle_ = camAlphaAngle_ - pix * abs(lastPos_.x() - pos.x() );
                
                
                if ( camAlphaAngle_ < 0 )
                    camAlphaAngle_ = - (abs(camAlphaAngle_) % 360); 
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
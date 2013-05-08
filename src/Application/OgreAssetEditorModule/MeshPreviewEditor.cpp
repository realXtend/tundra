// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "MeshPreviewEditor.h"
#include "OgreAssetEditorModule.h"
#include "LoggingFunctions.h"
#include "Application.h"
#include "Renderer.h"
#include "OgreRenderingModule.h"
#include "RenderWindow.h"
#include "IAsset.h"
#include "AssetAPI.h"
#include "IAssetTransfer.h"
#include "Math/MathConstants.h"

#include <QUiLoader>
#include <QFile>
#include <QLayout>
#include <QPushButton>
#include <QDebug>

#include "MemoryLeakCheck.h"

MeshPreviewLabel::MeshPreviewLabel(QWidget *parent, Qt::WindowFlags flags):
    QLabel(parent, flags)
{
    setMouseTracking(true);
}

MeshPreviewLabel::~MeshPreviewLabel()
{
}

void MeshPreviewLabel::mousePressEvent(QMouseEvent* ev)
{
    if (ev->buttons() == Qt::RightButton)
        sendMouseEvent(ev);
}

void MeshPreviewLabel::mouseReleaseEvent(QMouseEvent* ev)
{
//   if (ev->buttons() == Qt::RightButton)
    sendMouseEvent(ev);
}

void MeshPreviewLabel::wheelEvent(QWheelEvent* ev)
{
    sendWheelEvent(ev);
}

void MeshPreviewLabel::mouseMoveEvent(QMouseEvent *ev)
{
    if (ev->buttons() == Qt::RightButton)
        sendMouseEvent(ev);
}

MeshPreviewEditor::MeshPreviewEditor(const AssetPtr &meshAsset, Framework *framework, QWidget* parent): 
    QWidget(parent),
    framework_(framework),
    camAlphaAngle_(0),
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
    height_(400),
    asset(meshAsset)
{
    QUiLoader loader;
    QFile file(Application::InstallationDirectory() + "data/ui/mesh_preview.ui");
    if (!file.exists())
    {
        LogError("Cannot find OGRE Script Editor .ui file.");
        return;
    }

    mainWidget_ = loader.load(&file);
    file.close();

    setMouseTracking(true);

    QVBoxLayout* vLayout = mainWidget_->findChild<QVBoxLayout* >("imageLayout");
    
    label_ = new MeshPreviewLabel(mainWidget_);
    label_->setObjectName("meshImageLabel");

    label_->setMinimumSize(300,300);

    // Kind of hack, in this way we assure that we will get mouseMove-event with left and right button pressed.
    connect(label_, SIGNAL(sendMouseEvent(QMouseEvent*)), this, SLOT(MouseEvent(QMouseEvent*)));
    connect(label_, SIGNAL(sendWheelEvent(QWheelEvent*)), this, SLOT(MouseWheelEvent(QWheelEvent*)));
    vLayout->addWidget(label_);

    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);
    layout->addWidget(mainWidget_);
    layout->setContentsMargins(0, 0, 0, 0);

    SetMeshAsset(meshAsset);
}

MeshPreviewEditor::~MeshPreviewEditor()
{
    if (renderer_)
    {
        if (render_texture_)
            render_texture_->removeAllViewports();
        OgreRootPtr root = renderer_->OgreRoot();
        Ogre::TextureManager::getSingleton().remove(mesh_id_.toStdString().c_str());
        root_scene_->removeAllChildren();
        manager_->destroySceneNode(scene_);
        manager_->destroyCamera(camera_);
        manager_->destroyLight(newLight_);
        root->destroySceneManager(manager_);
    }
}

void MeshPreviewEditor::SetMeshAsset(const AssetPtr &meshAsset)
{
    asset = meshAsset;
    assert(asset.lock());
    AssetPtr assetPtr = asset.lock();
    if (!assetPtr)
        LogError("MeshPreviewEditor: null asset given.");
    if (assetPtr->Type() != "OgreMesh")
        LogWarning("Created MeshPreviewEditor for non-supported asset type " + assetPtr->Type() + ".");

    setWindowTitle(tr("Mesh: ") + (assetPtr?assetPtr->Name():QString()));

    // If asset is unloaded, load it now.
    if (assetPtr && !assetPtr->IsLoaded())
    {
        AssetTransferPtr transfer = framework_->Asset()->RequestAsset(assetPtr->Name(), assetPtr->Type(), true);
        connect(transfer.get(), SIGNAL(Succeeded(AssetPtr)), this, SLOT(OnAssetTransferSucceeded(AssetPtr)));
        connect(transfer.get(), SIGNAL(Failed(IAssetTransfer *, QString)), SLOT(OnAssetTransferFailed(IAssetTransfer *, QString)));
    }

    Open();
}

void MeshPreviewEditor::Open()
{
    if (asset.expired())
    {
        LogError("MeshPreviewEditor::Open: asset expired.");
        return;
    }

    AssetPtr assetPtr = asset.lock();
    mesh_id_ = AssetAPI::SanitateAssetRef(assetPtr->Name());
    Update();
}

void MeshPreviewEditor::Update()
{
    if (asset.expired())
        return;

    if (render_texture_ == 0 )
        CreateRenderTexture();

    try
    {
        // Hide the main UI Overlay, because otherwise Ogre will paint the Overlay onto the Mesh preview screen as well,
        // which is not desired.
        renderer_->GetRenderWindow()->ShowOverlay(false);

        AdjustScene();

        render_texture_->update();

        Ogre::Box bounds(0, 0, width_, height_);
        Ogre::uchar* pixelData = new Ogre::uchar[width_ * height_ * 4];
        Ogre::PixelBox pixels(bounds, Ogre::PF_A8R8G8B8, pixelData);
        
        render_texture_->copyContentsToMemory(pixels, Ogre::RenderTarget::FB_AUTO);

        // Create image of texture, and show it into label.

        u8* p = static_cast<u8 *>(pixels.data);
        uint width = (uint)pixels.getWidth();
        uint height = (uint)pixels.getHeight();

        QImage img = ConvertToQImage(p, width, height, 4);

        if(!img.isNull() && label_ != 0)
            label_->setPixmap(QPixmap::fromImage(img));

        delete[] pixelData;

        // Remember to re-enable the main UI now we're finished with the Ogre render.
        renderer_->GetRenderWindow()->ShowOverlay(true);
    }
    catch (const Ogre::Exception &e)
    {
        LogError("MeshPreviewEditor::Update: " + QString(e.what()));
    }
}

void MeshPreviewEditor::AdjustScene()
{
    if (scene_ == 0)
        return;

    scene_->rotate(Ogre::Vector3(0,1,0),Ogre::Radian(camAlphaAngle_ * pi/180.0));

    Ogre::AxisAlignedBox box = entity_->getBoundingBox();
    Ogre::Vector3 boxCenterPos = box.getHalfSize();
    Ogre::Vector3 volume = box.getSize();

    // Get biggest side :
    double biggest = boxCenterPos.x;
    if (biggest < boxCenterPos.y)
        biggest = boxCenterPos.y;
    if (biggest < boxCenterPos.z)
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
    try
    {
        renderer_ = framework_->GetModule<OgreRenderer::OgreRenderingModule>()->GetRenderer();

        // Create scene node and attach camera to it
        OgreRootPtr root = renderer_->OgreRoot();
        manager_ = root->createSceneManager(Ogre::ST_GENERIC, mesh_id_.toStdString().c_str());

        camera_ = manager_->createCamera(mesh_id_.toStdString().c_str());

        entity_ = manager_->createEntity("entity", mesh_id_.toStdString());
        entity_->setMaterialName("BaseWhite");

        Ogre::AxisAlignedBox box = entity_->getBoundingBox();
        Ogre::Vector3 boxCenterPos = box.getHalfSize();

        Ogre::Vector3 volume= box.getSize();

        // Get biggest side :

        double biggest = boxCenterPos.x;
        if (biggest < boxCenterPos.y )
            biggest = boxCenterPos.y;
        if (biggest < boxCenterPos.z )
            biggest = boxCenterPos.z;

        scene_ = manager_->createSceneNode(renderer_->GetUniqueObjectName(("MeshPreview_" + mesh_id_).toStdString()));
        scene_->attachObject(entity_);
        scene_->showBoundingBox(true);
        scene_->rotate(Ogre::Vector3(0,1,0),Ogre::Radian(camAlphaAngle_ * pi/180.0));

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
    }
    catch(const Ogre::Exception &e)
    {
        LogError("MeshPreviewEditor::CreateRenderTexture: " + QString(e.what()));
    }
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

    if (event->buttons() == Qt::RightButton)
    {
        if (pos.x() > lastPos_.x())
        {
            int width = label_->size().width();
            double pix = (2*360.0) / width;
            
            camAlphaAngle_ = camAlphaAngle_ + pix * abs(pos.x() - lastPos_.x());
            if (camAlphaAngle_ < 0)
                camAlphaAngle_ = - ( abs(camAlphaAngle_) % 360 ); 
            else
                camAlphaAngle_ = camAlphaAngle_ % 360;

            Update();
        }
        else if (pos.x() < lastPos_.x())
        {
            int width = label_->size().width();
            double pix = (2* 360.0 ) / width;

            camAlphaAngle_ = camAlphaAngle_ - pix * abs(lastPos_.x() - pos.x() );
            if (camAlphaAngle_ < 0)
                camAlphaAngle_ = - (abs(camAlphaAngle_) % 360); 
            else
                camAlphaAngle_ = camAlphaAngle_ % 360;

            Update();
        }
    }

   lastPos_ = pos;
}

QImage MeshPreviewEditor::ConvertToQImage(const u8 *raw_image_data, uint width, uint height, uint channels)
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

void MeshPreviewEditor::OnAssetTransferSucceeded(AssetPtr asset)
{
    Open();
}

void MeshPreviewEditor::OnAssetTransferFailed(IAssetTransfer *transfer, QString reason)
{
    LogError("MeshPreviewEditor::OnAssetTransferFailed: " + reason);
    //setText("Could not load asset: " + reason);
}


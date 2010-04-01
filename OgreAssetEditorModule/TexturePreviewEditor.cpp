#include "StableHeaders.h"
#include "Framework.h"
#include "DebugOperatorNew.h"
#include "TexturePreviewEditor.h"
#include "OgreAssetEditorModule.h"

#include <UiModule.h>
#include "Inworld/View/UiProxyWidget.h"
#include "Inworld/View/UiWidgetProperties.h"
#include "Inworld/InworldSceneController.h"
#include "ModuleManager.h"
#include "Inventory/InventoryEvents.h"
#include "AssetEvents.h"
#include "TextureInterface.h"
#include "ResourceInterface.h"
#include "TextureServiceInterface.h"

#include <QUiLoader>
#include <QFile>
#include <QPushButton>
#include <QLabel>
#include <QString>
#include <QLayout>
#include <QScrollArea>

#include "MemoryLeakCheck.h"

namespace Naali
{
    void TextureLabel::mousePressEvent(QMouseEvent *ev)
    {
        emit MouseClicked(ev);
    }

    TexturePreviewEditor::TexturePreviewEditor(Foundation::Framework *framework,
                                               const QString &inventory_id,
                                               const asset_type_t &asset_type,
                                               const QString &name,
                                               const QString &assetID,
                                               QWidget *parent):
    QWidget(parent),
    framework_(framework),
    inventoryId_(inventory_id),
    assetId_(assetID),
    assetType_(asset_type),
    mainWidget_(0),
    okButtonName_(0),
    headerLabel_(0),
    imageLabel_(0),
    scaleLabel_(0),
    layout_(0),
    request_tag_(0),
    imageSize_(QSize(0,0))
    {
        setObjectName(name);
        Initialize();
        RequestTextureAsset(assetId_);
    }

    TexturePreviewEditor::~TexturePreviewEditor()
    {

    }

    void TexturePreviewEditor::Closed()
    {
        boost::shared_ptr<UiServices::UiModule> ui_module =
            framework_->GetModuleManager()->GetModule<UiServices::UiModule>(Foundation::Module::MT_UiServices).lock();
        if (!ui_module.get())
            return;

        ui_module->GetInworldSceneController()->RemoveProxyWidgetFromScene(this);

        emit Closed(inventoryId_, assetType_);
    }

    void TexturePreviewEditor::RequestTextureAsset(const QString &asset_id)
    {
        Foundation::ServiceManagerPtr service_manager = framework_->GetServiceManager();
        if(service_manager)
        {
            if(service_manager->IsRegistered(Foundation::Service::ST_Texture))
            {
                boost::shared_ptr<Foundation::TextureServiceInterface> texture_service = 
                    service_manager->GetService<Foundation::TextureServiceInterface>(Foundation::Service::ST_Texture).lock();
                if(!texture_service)
                    return;

                // Request texture assets.
                request_tag_ = texture_service->RequestTexture(asset_id.toStdString());
            }
        }
    }

    void TexturePreviewEditor::HandleResouceReady(Resource::Events::ResourceReady *res)
    {
        if(request_tag_ == res->tag_)
        {
            Foundation::TextureInterface *tex = dynamic_cast<Foundation::TextureInterface *>(res->resource_.get());
            if(tex)
            {
                QImage img;
                if(tex->GetComponents() == 3)
                    img = QImage(tex->GetData(), tex->GetWidth(), tex->GetHeight(), QImage::Format_RGB888);
                else if(tex->GetComponents() == 4) 
                {
                    img = QImage(tex->GetData(), tex->GetWidth(), tex->GetHeight(), QImage::Format_ARGB32);
                    // No point to show chestboard elsewhere if image dont have alfa channel.
                    imageLabel_->setStyleSheet("QLabel#previewImageLabel"
                                               "{"
                                                    "background-image: url(\"./data/ui/images/image_background.png\");"
                                                    "background-repeat: repeat-xy;"
                                               "}");
                }
                else if(tex->GetComponents() == 1)
                    img = QImage(tex->GetData(), tex->GetWidth(), tex->GetHeight(), QImage::Format_Indexed8);

                if(!img.isNull())
                {
                    // Take image's original size.
                    imageSize_ = QSize(tex->GetWidth(), tex->GetHeight());
                    if(headerLabel_)
                        headerLabel_->setText(headerLabel_->text() + QString(" %1 x %2").arg(tex->GetWidth()).arg(tex->GetHeight()));

                    if(imageLabel_)
                    {
                        imageLabel_->setPixmap(QPixmap::fromImage(img));
                        UseTextureOriginalSize(true);
                        scaleLabel_->setText("Image scale: 1:1");
                    }
                }
            }
        }
    }

    void TexturePreviewEditor::TextureLabelClicked(QMouseEvent *ev)
    {
        if(ev->button() == Qt::LeftButton)
        {
            if(imageLabel_)
            {
                if(!scrollAreaWidget_->widgetResizable())
                {
                    UseTextureOriginalSize(false);
                    float scale = CalculateImageScale();
                    if(scale > 0)
                        scaleLabel_->setText(QString("Image scale: %1%").arg(scale));
                }
                else
                {
                    UseTextureOriginalSize(true);
                    scaleLabel_->setText("Image scale: 1:1");
                }
            }
            updateGeometry();
        }
    }

    void TexturePreviewEditor::resizeEvent(QResizeEvent *ev)
    {
        QWidget::resizeEvent(ev);
        float scale = CalculateImageScale();
        if(scale > 0)
            scaleLabel_->setText(QString("Image scale: %1%").arg(scale));
    }

    void TexturePreviewEditor::Initialize()
    {
        // Get QtModule and create canvas
        boost::shared_ptr<UiServices::UiModule> ui_module = 
            framework_->GetModuleManager()->GetModule<UiServices::UiModule>(Foundation::Module::MT_UiServices).lock();
        if (!ui_module.get())
            return;

        // Create widget from ui file
        QUiLoader loader;
        QFile file("./data/ui/texture_preview.ui");
        if (!file.exists())
        {
            OgreAssetEditorModule::LogError("Cannot find OGRE Script Editor .ui file.");
            return;
        }

        mainWidget_ = loader.load(&file);
        file.close();

        resize(cWindowMinimumWidth, cWindowMinimumHeight);

        layout_ = new QVBoxLayout;
        layout_->addWidget(mainWidget_);
        layout_->setContentsMargins(0, 0, 0, 0);
        setLayout(layout_);

        // Get controls
        okButtonName_ = mainWidget_->findChild<QPushButton *>("okButton");
        headerLabel_ = mainWidget_->findChild<QLabel *>("imageNameLabel");
        scaleLabel_ = mainWidget_->findChild<QLabel *>("imageScaleLabel");
        QLabel *assetIdLabel = mainWidget_->findChild<QLabel *>("imageAssetIdLabel");
        if(assetIdLabel)
            assetIdLabel->setText(assetId_);

        imageLabel_ = new TextureLabel();
        imageLabel_->setObjectName("previewImageLabel");
        imageLabel_->setScaledContents(true);
        imageLabel_->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
        QObject::connect(imageLabel_, SIGNAL(MouseClicked(QMouseEvent*)), this, SLOT(TextureLabelClicked(QMouseEvent*)));

        scrollAreaWidget_ = mainWidget_->findChild<QScrollArea *>("imageScrollArea");
        scrollAreaWidget_->widget()->layout()->addWidget(imageLabel_);
        //scrollAreaWidget_->setMinimumSize(QSize(cWindowMinimumWidth, cWindowMinimumHeight));

        // Set black background image that will be replaced once the real image has been received.
        QImage emptyImage = QImage(QSize(256, 256), QImage::Format_ARGB32);
        emptyImage.fill(qRgba(0,0,0,0));
        imageLabel_->setPixmap(QPixmap::fromImage(emptyImage));
        headerLabel_->setText(objectName());

        // Connect signals
        QObject::connect(okButtonName_, SIGNAL(clicked()), this, SLOT(Closed()));

        // Add widget to UI via ui services module
        UiServices::UiProxyWidget *proxy = ui_module->GetInworldSceneController()->AddWidgetToScene(this, UiServices::UiWidgetProperties("Texture: " + objectName(), UiServices::SceneWidget));
        QObject::connect(proxy, SIGNAL(Closed()), this, SLOT(Closed()));
        proxy->show();
        ui_module->GetInworldSceneController()->BringProxyToFront(proxy);
    }

    void TexturePreviewEditor::UseTextureOriginalSize(bool use)
    {
        if(use)
        {
            scrollAreaWidget_->setWidgetResizable(false);
            scrollAreaWidget_->widget()->resize(imageSize_);
            //Do we want to display the texture in it's real size or not.
            if(imageSize_.width() <= 512 && imageSize_.height() <= 512)
            {
                QSize mainWidgetSize = QSize(imageSize_.width() + 12, imageSize_.height() + 80);
                if(mainWidgetSize.width() > cWindowMinimumWidth)
                    mainWidgetSize.setWidth(mainWidgetSize.width());
                else
                    mainWidgetSize.setWidth(cWindowMinimumWidth);
                if(mainWidgetSize.height() > cWindowMinimumHeight)
                    mainWidgetSize.setHeight(mainWidgetSize.height());
                else
                    mainWidgetSize.setHeight(cWindowMinimumHeight);

                resize(mainWidgetSize);
                setMinimumSize(mainWidgetSize);
                setMaximumSize(mainWidgetSize);
            }
        }
        else
        {
            scrollAreaWidget_->setWidgetResizable(true);
            setMaximumSize(16777215, 16777215);
            setMinimumSize(0, 0);
        }
        updateGeometry();
    }

    float TexturePreviewEditor::CalculateImageScale()
    {
        if(imageLabel_)
        {
            long newImageSize = imageLabel_->size().width() * imageLabel_->size().height();
            long originalImageSize = imageSize_.width() * imageSize_.height();
            return (float(newImageSize) / float(originalImageSize)) * 100;
        }
        return -1.0f;
    }
}
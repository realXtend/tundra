/**
 *  For conditions of distribution and use, see copyright notice in LICENSE
 *
 *  @file   TexturePreviewEditor.cpp
 *  @brief  Preview window for textures.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "TexturePreviewEditor.h"
#include "OgreAssetEditorModule.h"
#include "TextureAsset.h"
#include "AssetAPI.h"
#include "IAssetTransfer.h"
#include "Application.h"
#include "LoggingFunctions.h"

#include <QUiLoader>
#include <QFile>
#include <QPushButton>
#include <QLabel>
#include <QString>
#include <QLayout>
#include <QScrollArea>

#include "MemoryLeakCheck.h"

// TextureLabel

TextureLabel::TextureLabel(QWidget *parent, Qt::WindowFlags flags):
    QLabel(parent, flags)
{
}

TextureLabel::~TextureLabel()
{
}

void TextureLabel::mousePressEvent(QMouseEvent *ev)
{
    emit MouseClicked(ev);
}

// TexturePreviewEditor

TexturePreviewEditor::TexturePreviewEditor(const AssetPtr &textureAsset, Framework *fw, QWidget* parent) :
    QWidget(parent),
    framework(fw),
    mainWidget_(0),
    headerLabel_(0),
    imageLabel_(0),
    scaleLabel_(0),
    layout_(0),
    imageSize_(QSize(0,0)),
    useOriginalImageSize_(true)
{
    QUiLoader loader;
    QFile file(Application::InstallationDirectory() + "data/ui/texture_preview.ui");
    if (!file.exists())
    {
        LogError("Cannot find Texture Preview Editor .ui file.");
        return;
    }

    mainWidget_ = loader.load(&file);
    file.close();

    resize(cWindowMinimumWidth, cWindowMinimumHeight);

    layout_ = new QVBoxLayout;
    layout_->addWidget(mainWidget_);
    layout_->setContentsMargins(0, 0, 0, 0);
    setLayout(layout_);

    headerLabel_ = mainWidget_->findChild<QLabel *>("imageNameLabel");
    scaleLabel_ = mainWidget_->findChild<QLabel *>("imageScaleLabel");

    QLabel *assetIdLabel = mainWidget_->findChild<QLabel *>("imageAssetIdLabel");
    if (assetIdLabel)
        assetIdLabel->setText("");
    
    imageLabel_ = new TextureLabel();
    imageLabel_->setObjectName("previewImageLabel");
    imageLabel_->setScaledContents(true);
    imageLabel_->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    connect(imageLabel_, SIGNAL(MouseClicked(QMouseEvent*)), this, SLOT(TextureLabelClicked(QMouseEvent*)));

    scrollAreaWidget_ = mainWidget_->findChild<QScrollArea *>("imageScrollArea");
    scrollAreaWidget_->widget()->layout()->addWidget(imageLabel_);

    // Set black background image that will be replaced once the real image has been received.
    QImage emptyImage = QImage(QSize(256, 256), QImage::Format_ARGB32);
    emptyImage.fill(qRgba(0,0,0,0));
    imageLabel_->setPixmap(QPixmap::fromImage(emptyImage));
    headerLabel_->setText(objectName());

    SetTextureAsset(textureAsset);
}

TexturePreviewEditor::~TexturePreviewEditor()
{
}

void TexturePreviewEditor::SetTextureAsset(const AssetPtr &textureAsset)
{
    asset = textureAsset;
    assert(asset.lock());
    AssetPtr assetPtr = asset.lock();
    setWindowTitle(tr("Texture: ") + (assetPtr?assetPtr->Name():QString()));
    if (!assetPtr)
        LogError("TexturePreviewEditor: null asset given.");
    if (assetPtr && assetPtr->Type() != "Texture")
        LogWarning("Created TexturePreviewEditor for non-supported asset type " + assetPtr->Type() + ".");

    // If asset is unloaded, load it now.
    if (assetPtr && !assetPtr->IsLoaded())
    {
        AssetTransferPtr transfer = framework->Asset()->RequestAsset(assetPtr->Name(), assetPtr->Type(), true);
        connect(transfer.get(), SIGNAL(Succeeded(AssetPtr)), this, SLOT(OnAssetTransferSucceeded(AssetPtr)));
        connect(transfer.get(), SIGNAL(Failed(IAssetTransfer *, QString)), SLOT(OnAssetTransferFailed(IAssetTransfer *, QString)));
    }
    else
        Open();
}
///\todo Move the code below to Open().
/*
void TexturePreviewEditor::HandleResouceReady()
{
    TextureAsset *tex = dynamic_cast<TextureAsset *>(asset.get());
    if (!tex)
    {
        LogError();
        return;
    }

    QImage img = ConvertToQImage(tex->GetData(), tex->GetWidth(), tex->GetHeight(), tex->GetComponents());
    // Only show chessboard pattern if image has an alpha channel.
    if (tex->GetComponents() == 4 || tex->GetComponents() == 2) 
    {
        imageLabel_->setStyleSheet(
            "QLabel#previewImageLabel"
            "{"
                "background-image: url(\Application::InstallationDirectory + "data/ui/images/image_background.png\");"
                "background-repeat: repeat-xy;"
            "}");
    }

    if (!img.isNull())
    {
        // Take image's original size.
        imageSize_ = QSize(tex->GetWidth(), tex->GetHeight());
        if (headerLabel_)
            headerLabel_->setText(headerLabel_->text() + QString(" %1 x %2").arg(tex->GetWidth()).arg(tex->GetHeight()));

        if (imageLabel_)
        {
            imageLabel_->setPixmap(QPixmap::fromImage(img));
            UseTextureOriginalSize(true);
            scaleLabel_->setText("Image scale: 1:1");
        }
    }
}
*/

void TexturePreviewEditor::TextureLabelClicked(QMouseEvent *ev)
{
    if(ev->button() == Qt::LeftButton)
    {
        if(imageLabel_)
        {
            if(useOriginalImageSize_)
            {
                UseTextureOriginalSize(false);
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
    if(!useOriginalImageSize_ && scrollAreaWidget_)
    {
        if(scrollAreaWidget_->widget())
        {
            // Scale image but keep it's aspect ratio same.
            float pixmapAspectRatio[2];
            pixmapAspectRatio[0] = float(imageSize_.width()) / float(imageSize_.height());
            pixmapAspectRatio[1] = float(imageSize_.height()) / float(imageSize_.width());
            QSize newImageSize;
            // TODO Brutal force approach might be too heavy.
            for(uint i = 0; i < 2; i++)
            {
                if(i == 0)
                    newImageSize = QSize((scrollAreaWidget_->size().height() - 4) * pixmapAspectRatio[i], (scrollAreaWidget_->size().height() - 4));
                else
                    newImageSize = QSize((scrollAreaWidget_->size().width() - 4), (scrollAreaWidget_->size().width() - 4) * pixmapAspectRatio[i]);
                //Check if image fits into scroll area and if it does no need to continue.
                if(newImageSize.width() <= scrollAreaWidget_->size().width() && newImageSize.height() <= scrollAreaWidget_->size().height())
                {
                    scrollAreaWidget_->widget()->resize(newImageSize);
                    break;
                }
            }
        }
        float scale = CalculateImageScale();
        if(scale > 0)
            scaleLabel_->setText(QString("Image scale: %1%").arg(scale));
    }
}

void TexturePreviewEditor::Open()
{
    if (asset.expired())
    {
        LogError("TexturePreviewEditor::Open: Texture asset expired.");
        return;
    }

    AssetPtr assetPtr = asset.lock();
    Ogre::ResourcePtr res = Ogre::TextureManager::getSingleton().getByName(AssetAPI::SanitateAssetRef(assetPtr->Name()).toStdString());
    Ogre::Texture* tex = dynamic_cast<Ogre::Texture* >(res.get());
    if (!tex)
    {
        LogWarning("Failed to open Ogre texture " + assetPtr->Name() + " .");
        return;
    }

    // Create image of texture, and show it into label.
    QImage img = TextureAsset::ToQImage(tex);

    if(!img.isNull() && imageLabel_ != 0)
    {
        imageLabel_->setPixmap(QPixmap::fromImage(img));
        imageLabel_->show();
        
        QLabel *assetIdLabel = mainWidget_->findChild<QLabel *>("imageAssetIdLabel");
        if (assetIdLabel)
            assetIdLabel->setText(AssetAPI::DesanitateAssetRef(assetPtr->Name()));
    }
}

void TexturePreviewEditor::UseTextureOriginalSize(bool use)
{
    if(use) //Set texture to it's real size.
    {
        scrollAreaWidget_->setWidgetResizable(false);
        scrollAreaWidget_->widget()->resize(imageSize_);
        scrollAreaWidget_->widget()->setMinimumSize(imageSize_);
        scrollAreaWidget_->widget()->setMaximumSize(imageSize_);
        //Do we want to display the texture in it's real size or not. if texture's size is over 512 x 512 we will stay 
        //at smaller window size cause that texture would take too much screen space.
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
        }
        useOriginalImageSize_ = true;
    }
    else //scale texture based on it's parent widget size.
    {
        scrollAreaWidget_->setWidgetResizable(false);
        scrollAreaWidget_->widget()->setMaximumSize(16777215, 16777215);
        scrollAreaWidget_->widget()->setMinimumSize(0, 0);
        scrollAreaWidget_->widget()->resize(imageSize_);
        useOriginalImageSize_ = false;
    }

    float scale = CalculateImageScale();
    if(scale > 0)
        scaleLabel_->setText(QString("Image scale: %1%").arg(scale));
}

float TexturePreviewEditor::CalculateImageScale()
{
    if(imageLabel_)
    {
        long newImageSize = imageLabel_->size().width() * imageLabel_->size().height();
        long originalImageSize = imageSize_.width() * imageSize_.height();
        if(newImageSize > 0 && originalImageSize > 0)
            return (float(newImageSize) / float(originalImageSize)) * 100;
    }
    return -1.0f;
}

/*
QImage TexturePreviewEditor::ConvertToQImage(const u8 *raw_image_data, uint width, uint height, uint channels)
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
*/
void TexturePreviewEditor::OnAssetTransferSucceeded(AssetPtr asset)
{
    SetTextureAsset(asset);
}

void TexturePreviewEditor::OnAssetTransferFailed(IAssetTransfer *transfer, QString reason)
{
    LogError("TexturePreviewEditor::OnAssetTransferFailed: " + reason);
    //setText("Could not load asset: " + reason);
}

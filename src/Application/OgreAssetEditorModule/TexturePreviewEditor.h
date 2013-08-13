/**
 *  For conditions of distribution and use, see copyright notice in LICENSE
 *
 *  @file   TexturePreviewEditor.h
 *  @brief  Preview window for textures.
 */

#pragma once

#include "CoreTypes.h"
#include "OgreAssetEditorModuleApi.h"
#include "AssetFwd.h"

#include <QWidget>
#include <QLabel>

class QPushButton;
class QScrollArea;

/// TextureLabel widget is used only in texture preview window.
/** Widget is listening it's mousePressEvents and will send a signal when user will click the label.
    @cond PRIVATE */
class TextureLabel: public QLabel
{
    Q_OBJECT
public:
    TextureLabel(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    virtual ~TextureLabel();

signals:
    /// When user click this widget it will send a signal.
    void MouseClicked(QMouseEvent *ev);

protected:
    /// Override mousePressEvent and emit a signal when user is clicking the widget.
    virtual void mousePressEvent(QMouseEvent *ev);
};
/** @endcond */

/// Preview window for textures.
/** Will convert texture asset into Qt image format and display the image in image label. */
class ASSET_EDITOR_MODULE_API TexturePreviewEditor: public QWidget
{
    Q_OBJECT

public:
    TexturePreviewEditor(const AssetPtr &textureAsset, Framework *fw, QWidget* parent = 0);
    virtual ~TexturePreviewEditor();

    /// Opens text texture
    void Open();

    /// Sets the texture for viewing.
    void SetTextureAsset(const AssetPtr &textureAsset);

public slots:
    /// Listens when image label has been pressed.
    void TextureLabelClicked(QMouseEvent *ev);

protected:
    /// overrides QWidget's resizeEvent so we can recalculate new image differences.
    virtual void resizeEvent(QResizeEvent *ev);

private:
    /// Will the texture displayed as original size or not.
    void UseTextureOriginalSize(bool use = true);

    /// Compare original image and current image size and return their size difference in percents.
    /// @return images size difference in percents if something went wrong return -1.
    float CalculateImageScale();

    /// This method is used to convert the decoded raw image data into QImage format.
    /** 1 channel image is converted to RGB888 image-format so that all channels have the same value.
        2 channel image is converted to ARGB32 image-format all where 1 channel is grayscale value and 2 channel is alpha channel.
        3 channel image is converted to RGB888 format as normal.
        4 channel image is converted to ARGB32 format.

        @param raw_image_data is pointer to image data.
        @param width tell image's width.
        @param height tell image's height.
        @param channels tell how many image channels is in use.
        @return QImage and if fail return empty image.
    QImage ConvertToQImage(const u8 *raw_image_data, uint width, uint height, uint channels = 3);
    */

    ///\todo Remove these two when have more time in hand.
    const static int cWindowMinimumWidth = 256;
    const static int cWindowMinimumHeight = 323;

    Framework *framework;
    AssetWeakPtr asset;
    QWidget *mainWidget_;
    QScrollArea *scrollAreaWidget_;
    QLayout *layout_;
    QPushButton *okButtonName_;
    QLabel *headerLabel_;
    QLabel *scaleLabel_;
    TextureLabel *imageLabel_;
    QSize imageSize_;
    QString assetId_;
    bool useOriginalImageSize_;

private slots:
    void OnAssetTransferSucceeded(AssetPtr asset);
    void OnAssetTransferFailed(IAssetTransfer *transfer, QString reason);
};

// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file   TexturePreviewEditor.h
 *  @brief  Preview window for uploaded textures.
 *          Will convert texture asset into Qt image format and display the image in image label.
 */

#ifndef incl_OgreAssetEditorModule_TexturePreviewEditor_h
#define incl_OgreAssetEditorModule_TexturePreviewEditor_h

#include <RexTypes.h>
#include <QWidget>
#include <QLabel>

QT_BEGIN_NAMESPACE
class QPushButton;
class QLabel;
class QString;
class QScrollArea;
QT_END_NAMESPACE

namespace Resource
{
    namespace Events
    {
        class ResourceReady;
    }
}

namespace Foundation
{
    class Framework;
    class AssetInterface;
    typedef boost::shared_ptr<AssetInterface> AssetPtr;
}

namespace Naali
{
    //! TextureLabel widget is used only in texture preview window.
    //! Widget is listening it's mousePressEvents and will send a signal when user will click the label.
    //! 
    class TextureLabel: public QLabel
    {
        Q_OBJECT
    public:
        TextureLabel(QWidget *parent = 0, Qt::WindowFlags flags = 0);
        virtual ~TextureLabel();
    signals:
        //! When user click this widget it will send a signal.
        void MouseClicked(QMouseEvent *ev);
    protected:
        //! Override mousePressEvent and emit a signal when user is clicking the widget.
        virtual void mousePressEvent(QMouseEvent *ev);
    };

    class TexturePreviewEditor: public QWidget
    {
        Q_OBJECT
    public:
        //TODO! Remove those two when have more time inhand.
        const static int cWindowMinimumWidth = 256;
        const static int cWindowMinimumHeight = 323;

        TexturePreviewEditor(Foundation::Framework *framework,
                             const QString &inventory_id,
                             const asset_type_t &asset_type,
                             const QString &name,
                             const QString &asset_id,
                             QWidget *parent = 0);
        virtual ~TexturePreviewEditor();

        TexturePreviewEditor(Foundation::Framework *framework, QWidget* parent = 0);

    public slots:
        /// Close the window.
        void Closed();
        /// Request texture asset from texture decoder service.
        void RequestTextureAsset(const QString &asset_id);

        //! handles resouce ready event and if tag fits, convert it into QImage format and set it to image label.
        void HandleResouceReady(Resource::Events::ResourceReady *res);

        //! Listenes when image label has been pressed.
        void TextureLabelClicked(QMouseEvent *ev);

    signals:
        /// This signal is emitted when the editor is closed.
        void Closed(const QString &inventory_id, asset_type_t asset_type);

    protected:
        //! overrides QWidget's resizeEvent so we can recalculate new image differences.
        virtual void resizeEvent(QResizeEvent *ev);

    private:
        void Initialize();
        //! Will the texture displayed as original size or not.
        void UseTextureOriginalSize(bool use = true);
        //! Compare original image and current image size and return their size difference in percents.
        //! @return images size defference in percents if something went wrong return -1.
        float CalculateImageScale();

        //! This method is used to convert the decoded raw image data into QImage format.
        //! 1 channel image is converted to RGB888 image-format so that all channels have the same value.
        //! 2 channel image is converted to ARGB32 image-format all where 1 channel is grayscale value and 2 channel is alfa channel.
        //! 3 channel image is converted to RGB888 format as normal.
        //! 4 channel image is converted to ARGB32 format.
        //! @param raw_image_data is pointer to image data.
        //! @param width tell image's width.
        //! @param height tell image's height.
        //! @param channels tell how many image channels is in use.
        //! @return QImage and if fail return empty image.
        QImage ConvertToQImage(const u8 *raw_image_data, int width, int height, int channels = 3);

        Foundation::Framework *framework_;
        QWidget *mainWidget_;
        QScrollArea *scrollAreaWidget_;
        QLayout *layout_;
        QPushButton *okButtonName_;
        QLabel *headerLabel_;
        QLabel *scaleLabel_;
        TextureLabel *imageLabel_;
        QSize imageSize_;

        QString inventoryId_;
        QString assetId_;
        asset_type_t assetType_;
        request_tag_t request_tag_;
        bool useOriginalImageSize_;
    };
}

#endif

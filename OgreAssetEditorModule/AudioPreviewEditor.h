// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file   AudioPreviewEditor.h
 *  @brief  Preview window for uploaded audio clips.
 *          Window is used to play audio assets and display the infomation about that file, including the visual presentation of audio signal.
 */

#ifndef incl_OgreAssetEditorModule_AudioPreviewEditor_h
#define incl_OgreAssetEditorModule_AudioPreviewEditor_h

#include <RexTypes.h>
#include <QWidget>

#include <boost/shared_ptr.hpp>

QT_BEGIN_NAMESPACE
class QPushButton;
class QTimer;
QT_END_NAMESPACE

namespace Foundation
{
    class Framework;
    class AssetInterface;
    typedef boost::shared_ptr<AssetInterface> AssetPtr;
}

namespace Resource
{
    namespace Events
    {
        class ResourceReady;
    }
}

//! AudioPreviewEditor is used to play different audioclips from the inventory and display a audio info diagram.
class AudioPreviewEditor: public QWidget
{
    Q_OBJECT
public:
    AudioPreviewEditor(Foundation::Framework *framework,
                       const QString &inventory_id,
                       const asset_type_t &asset_type,
                       const QString &name,
                       QWidget *parent = 0);
    virtual ~AudioPreviewEditor();

    void HandleAssetReady(Foundation::AssetPtr asset);

    void HandleResouceReady(Resource::Events::ResourceReady *res);

public slots:
    /// Close the window.
    void Closed();

    //! Toggle between sound play and stop states.
    void PlaySound();
    void TimerTimeout();

signals:
    //! This signal is emitted when the editor is closed.
    void Closed(const QString &inventory_id, asset_type_t asset_type);

    //! Signal for widget resize.
    void WidgetResized(QSize size);

protected:
    virtual void resizeEvent(QResizeEvent *ev);

private:
    void InitializeEditorWidget();

    Foundation::Framework *framework_;
    asset_type_t assetType_;
    request_tag_t request_tag_;
    QString inventoryId_;
    QString assetId_;
    sound_id_t soundId_;

    QWidget     *mainWidget_;
    QPushButton *okButton_;
    QPushButton *playButton_;
    QTimer      *playTimer_;
};

#endif

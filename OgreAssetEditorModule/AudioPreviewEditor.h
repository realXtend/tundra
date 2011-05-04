// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file   AudioPreviewEditor.h
 *  @brief  Preview window for uploaded audio clips.
 *          Window is used to play audio assets and display the infomation about that file, including the visual presentation of audio signal.
 */

#ifndef incl_OgreAssetEditorModule_AudioPreviewEditor_h
#define incl_OgreAssetEditorModule_AudioPreviewEditor_h


#include <QWidget>

#include "AssetFwd.h"

#include <boost/shared_ptr.hpp>

class QPushButton;
class QTimer;

class Framework;

/// AudioPreviewEditor is used to play different audioclips from the inventory and display a audio info diagram.
class AudioPreviewEditor: public QWidget
{
    Q_OBJECT
public:
    AudioPreviewEditor(Framework *framework, const QString &name, QWidget *parent = 0);
    virtual ~AudioPreviewEditor();

public slots:
    /// Close the window.
    void Closed();

    /// Toggle between sound play and stop states.
    void PlaySound();
    void TimerTimeout();

signals:
    /// This signal is emitted when the editor is closed.
    void Closed(const QString &inventory_id);

    /// Signal for widget resize.
    void WidgetResized(QSize size);

protected:
    virtual void resizeEvent(QResizeEvent *ev);

private:
    void InitializeEditorWidget();

    Framework *framework_;

    QString assetId_;

    QWidget     *mainWidget_;
    QPushButton *okButton_;
    QPushButton *playButton_;
    QTimer      *playTimer_;
};

#endif

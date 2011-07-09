/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   AudioPreviewEditor.h
 *  @brief  Preview window for audio assets.
 */

#pragma once

#include <QWidget>

#include "AssetFwd.h"

#include <boost/shared_ptr.hpp>

class QPushButton;
class QTimer;

class Framework;

/// Preview window for audio assets.
/** Window is used to play audio assets and display the infomation about that file, including the visual presentation of audio signal. */
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
    Framework *framework_;
    QString assetId_;
    QWidget *mainWidget_;
    QPushButton *okButton_;
    QPushButton *playButton_;
    QTimer *playTimer_;
};

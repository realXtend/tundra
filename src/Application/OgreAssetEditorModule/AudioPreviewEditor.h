/**
 *  For conditions of distribution and use, see copyright notice in LICENSE
 *
 *  @file   AudioPreviewEditor.h
 *  @brief  Preview window for audio assets.
 */

#pragma once

#include <QWidget>

#include "AssetFwd.h"
#include "AudioFwd.h"

class QPushButton;
class QTimer;

class Framework;

/// Preview window for audio assets.
/** Window is used to play audio assets and display the infomation about that file, including the visual presentation of audio signal. */
class AudioPreviewEditor : public QWidget
{
    Q_OBJECT

public:
    AudioPreviewEditor(const AssetPtr &audioAsset, Framework *fw, QWidget *parent = 0);
    virtual ~AudioPreviewEditor();

public slots:
    /// Toggle between sound play and stop states.
    void PlaySound();
    void TimerTimeout();

private:
//    void DrawAudioSignal();

    Framework *framework_;
    AssetWeakPtr asset;
    SoundChannelPtr soundChannel;
    QWidget *mainWidget_;
    QPushButton *okButton_;
    QPushButton *playButton_;
    QTimer *playTimer_;

private slots:
    void OnAssetTransferSucceeded(AssetPtr asset);
    void OnAssetTransferFailed(IAssetTransfer *transfer, QString reason);
};

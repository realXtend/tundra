// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include "VlcFwd.h"
#include "PlayerStatus.h"
#include "ui_MediaPlayer.h"

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QSlider>
#include <QUrl>

class VlcMediaPlayer : public QWidget
{

Q_OBJECT

public:
    /// Constructor
    VlcMediaPlayer();

    /// Deconstructor
    ~VlcMediaPlayer();

public slots:
    /// Load media source.
    /// @param QString source
    /// @return boolean True if format is supported and loaded, false other wise.
    bool LoadMedia(const QString &source);

    /// Load media source.
    /// @param QUrl source
    /// @return boolean True if format is supported and loaded, false other wise.
    bool LoadMedia(const QUrl &source);

    /// Returns the current media.
    QString Media();

    /// Ui callback for play/pause button
    bool PlayPause();

    /// Ui callback for stop button
    void Stop();

    /// Ui callback for time slider
    void Seek();

    /// Force to emit the idle image
    void ForceUpdateImage();

signals:
    /// Rendering frame update.
    void FrameUpdate(QImage frame);

private slots:
    // Initialized status check
    bool Initialized();

    /// Slot for player status
    void OnStatusUpdate(const PlayerStatus &status);

    /// Generate media player startup parameters
    const QList<QByteArray> GenerateVlcParameters();

private:
    VlcPlugin *vlcPlugin_;
    VlcVideoWidget *videoWidget_;
    Ui::MediaPlayer ui_;

    QVBoxLayout *mainLayout_;
    QPushButton *playButton_;
    QPushButton *pauseButton_;
    QPushButton *stopButton_;
    QSlider *timeSlider_;
    QLabel *timeLabel_;

    QString totalTime_;
    QString nowTime_;
    QString currentSource_;
};
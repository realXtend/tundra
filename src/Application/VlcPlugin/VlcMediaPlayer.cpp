// For conditions of distribution and use, see copyright notice in license.txt

#include "VlcMediaPlayer.h"
#include "VlcVideoWidget.h"
#include "Application.h"
#include "LoggingFunctions.h"

#include <QDir>
#include <QFile>
#include <QLatin1Literal>
#include <QTime>

#include <QDebug>

VlcMediaPlayer::VlcMediaPlayer() :
    QWidget(0),
    videoWidget_(0),
    currentSource_("")
{
    ui_.setupUi(this);
    ui_.pauseButton->hide();

    try
    {
        videoWidget_ = new VlcVideoWidget(GenerateVlcParameters());

        connect(videoWidget_, SIGNAL(StatusUpdate(const PlayerStatus)), SLOT(OnStatusUpdate(const PlayerStatus)));
        connect(videoWidget_, SIGNAL(FrameUpdate(QImage)), SIGNAL(FrameUpdate(QImage)), Qt::QueuedConnection);
        
        connect(ui_.playButton, SIGNAL(clicked()), SLOT(PlayPause()));
        connect(ui_.pauseButton, SIGNAL(clicked()), SLOT(PlayPause()));
        connect(ui_.stopButton, SIGNAL(clicked()), SLOT(Stop()));
        connect(ui_.timeSlider, SIGNAL(sliderReleased()), SLOT(Seek()));
        
        ui_.verticalLayout->insertWidget(0, videoWidget_, 2);
    }
    catch (std::exception &e)
    {
        LogError(QString("VlcMediaPlayer: ") + e.what());
    }

    setWindowTitle("");
    hide();
}

VlcMediaPlayer::~VlcMediaPlayer()
{
    if (videoWidget_)
        videoWidget_->ShutDown();
}

// Public slots

bool VlcMediaPlayer::LoadMedia(const QUrl &source)
{
    return LoadMedia(source.toString());
}

bool VlcMediaPlayer::LoadMedia(const QString &source)
{
    if (!Initialized())
    {
        LogInfo("VlcMediaPlayer: Cannot play back media, VLC not initialized");
        return false;
    }

    return videoWidget_->OpenSource(source);
}

// Private slots

bool VlcMediaPlayer::Initialized()
{
    if (!videoWidget_ || !videoWidget_->Initialized())
        return false;
    return true;
}

bool VlcMediaPlayer::PlayPause()
{
    if (!Initialized())
        return false;

    bool playing = videoWidget_->TogglePlay();
    ui_.playButton->setVisible(!playing);
    ui_.pauseButton->setVisible(playing);
    return playing;
}

void VlcMediaPlayer::Stop()
{
    if (!Initialized())
        return;

    videoWidget_->Stop();
    ui_.timeSlider->setValue(0);
    ui_.timeSlider->setEnabled(false);
    ui_.playButton->setVisible(true);
    ui_.pauseButton->setVisible(false);
    ui_.timeLabel->setText("0:0" + totalTime_);
}

void VlcMediaPlayer::Seek()
{
    if (!Initialized())
        return;

    videoWidget_->Seek(ui_.timeSlider->value());
}

void VlcMediaPlayer::ForceUpdateImage()
{
    if (!Initialized())
        return;

    videoWidget_->ForceUpdateImage();
}

void VlcMediaPlayer::OnStatusUpdate(const PlayerStatus status)
{
    switch (status.change)
    {
        case PlayerStatus::MediaState:
        {
            if (status.stopped)
            {
                ui_.timeSlider->setValue(0);
                ui_.timeSlider->setEnabled(false);
                ui_.playButton->setVisible(true);
                ui_.pauseButton->setVisible(false);
            }
            if (status.playing)
            {
                ui_.timeSlider->setEnabled(true);
                ui_.playButton->setVisible(!status.playing);
                ui_.pauseButton->setVisible(status.playing);
            }
            if (status.paused)
            {
                ui_.timeSlider->setEnabled(false);
                ui_.playButton->setVisible(status.paused);
                ui_.pauseButton->setVisible(!status.paused);
            }
            ForceUpdateImage();
            break;
        }
        case PlayerStatus::MediaTime:
        {
            if (!ui_.timeSlider->isSliderDown())
                ui_.timeSlider->setValue(status.time);
            if (ui_.timeSlider->maximum() != status.lenght)
            {
                QTime tTotal;
                tTotal = tTotal.addMSecs(status.lenght);
                totalTime_ =  " / " + tTotal.toString((tTotal.hour() > 0 ? "H:m:ss" : "m:ss"));
                ui_.timeSlider->setRange(0, status.lenght);
            }
            QTime tNow;
            tNow = tNow.addMSecs(status.time);
            nowTime_ = tNow.toString((tNow.hour() > 0 ? "H:m:ss" : "m:ss"));
            ui_.timeLabel->setText(nowTime_ + totalTime_);
            break;
        }
        case PlayerStatus::MediaSource:
        {
            currentSource_ = status.source;
            setWindowTitle(status.source);
            ui_.playButton->setVisible(true);
            ui_.pauseButton->setVisible(false);
            break;
        }
        case PlayerStatus::PlayerError:
        {
            LogError(status.error);
            break;
        }
        case PlayerStatus::MediaSize:
            break;
        case PlayerStatus::MediaProperty:
            break;
        case PlayerStatus::NoChange:
            break;
    }
}

const QList<QByteArray> VlcMediaPlayer::GenerateVlcParameters()
{
    QString folderToFind = "vlcplugins";
    QDir pluginDir(Application::InstallationDirectory());

    if (pluginDir.exists(folderToFind))
        pluginDir.cd(folderToFind);
    else
    {
        // This most likely means we are running 
        // inside a IDE from the viewer projects folder.
        // as in tundra/Viewer/{RelWithDebInfo|Debug|Release}
        while (!pluginDir.exists("bin"))
        {
            if (!pluginDir.cdUp())
                throw std::exception("Fatal error, could not find vlc plugins path!");
        }
        pluginDir.cd("bin");
        pluginDir.cd(folderToFind);
    }

    // Validate
    if (!pluginDir.absolutePath().endsWith(folderToFind))
        throw std::exception("Fatal error, could not find vlc plugins path!");

    // Set plugin path
    QString pluginPath = QLatin1Literal("--plugin-path=") % QDir::toNativeSeparators(pluginDir.absolutePath());

    QList<QByteArray> params;
    params << QFile::encodeName(pluginPath);    // Plugin path
    params << QByteArray("--intf=dummy");       // No interface
    return params;
}

QString VlcMediaPlayer::Media()
{
    return currentSource_;
}

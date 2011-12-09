// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "AudioPreviewEditor.h"
#include "AudioSignalLabel.h"
#include "OgreAssetEditorModule.h"

#include "Framework.h"
#include "Application.h"
#include "IAsset.h"
#include "AudioAPI.h"
#include "LoggingFunctions.h"
#include "AssetAPI.h"
#include "IAsset.h"
#include "IAssetTransfer.h"

#include <QUiLoader>
#include <QFile>
#include <QLayout>
#include <QPushButton>
#include <QImage>
#include <QTimer>

#include "MemoryLeakCheck.h"

AudioPreviewEditor::AudioPreviewEditor(const AssetPtr &audioAsset, Framework *fw, QWidget *parent) :
    QWidget(parent),
    framework_(fw),
    okButton_(0),
    playButton_(0),
    playTimer_(0),
    mainWidget_(0),
    asset(audioAsset)
{
    assert(asset.lock());
    AssetPtr assetPtr = asset.lock();
    if (!assetPtr)
        LogError("AudioPreviewEditor: null asset given.");
    if (assetPtr->Type() != "OgreMaterial" && assetPtr->Type() != "OgreParticle")
        LogWarning("Created AudioPreviewEditor for non-supported asset type " + assetPtr->Type() + ".");

    // Create widget from ui file
    QUiLoader loader;
    QFile file(Application::InstallationDirectory() + "data/ui/audio_preview.ui");
    if (!file.exists())
    {
        LogError("Cannot find OGRE Script Editor .ui file.");
        return;
    }
    mainWidget_ = loader.load(&file);
    file.close();

    resize(mainWidget_->size());

    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);
    layout->addWidget(mainWidget_);
    layout->setContentsMargins(0, 0, 0, 0);

//    okButton_ = mainWidget_->findChild<QPushButton *>("okButton");

    playButton_ = mainWidget_->findChild<QPushButton *>("playButton");
    connect(playButton_, SIGNAL(clicked()), this, SLOT(PlaySound()));

    setWindowTitle(tr("Audio: ") + (assetPtr?assetPtr->Name():QString()));

    // If asset is unloaded, load it now.
    if (assetPtr && !assetPtr->IsLoaded())
    {
        AssetTransferPtr transfer = framework_->Asset()->RequestAsset(assetPtr->Name(), assetPtr->Type(), true);
        connect(transfer.get(), SIGNAL(Succeeded(AssetPtr)), this, SLOT(OnAssetTransferSucceeded(AssetPtr)));
        connect(transfer.get(), SIGNAL(Failed(IAssetTransfer *, QString)), SLOT(OnAssetTransferFailed(IAssetTransfer *, QString)));
    }
}

AudioPreviewEditor::~AudioPreviewEditor()
{
}

/*
void AudioPreviewEditor::DrawAudioSignal()
{
    AudioAsset *sound = dynamic_cast<AudioAsset *>(asset.lock().get());
    if (sound)
    {
        QLabel *audioInfoLabel = findChild<QLabel*>("descriptionLabel");
        //SoundBuffer buffer = sound->GetBuffer();
        SoundBuffer buffer;
        audioInfoLabel->setText(QString(tr("Frequency: %1Hz")).arg(buffer.frequency));
        
        int bits = 16;
        if (!buffer.is16Bit)
            bits = 8;
        audioInfoLabel->setText(audioInfoLabel->text() + QString(tr(" Bits: %1 bit")).arg(bits));

        QString stereo = "Stereo";
        if(!buffer.stereo)
            stereo = "Mono";
        audioInfoLabel->setText(audioInfoLabel->text() + tr(" Format: ") + stereo);

        float duration;
        if (!buffer.stereo)
            duration = float(buffer.data.size() / ((bits / 8))) / float(buffer.frequency);
        else
            duration = float(buffer.data.size() / ((bits / 8) * 2)) / float(buffer.frequency);
        audioInfoLabel->setText(audioInfoLabel->text() + QString(tr("\nDuration: %1 sec")).arg(duration));

        QVBoxLayout *layout = mainWidget_->findChild<QVBoxLayout*>("verticalLayout_2");
        if (layout)
        {
            AudioSignalLabel *audioSignalLabel = mainWidget_->findChild<AudioSignalLabel *>("audioSignalLabel");
            if (!audioSignalLabel)
            {
                audioSignalLabel = new AudioSignalLabel(this);
                audioSignalLabel->setObjectName("audioSignalLabel");
                layout->addWidget(audioSignalLabel);
            }
            audioSignalLabel->SetAudioData(buffer.data, buffer.frequency, bits, buffer.stereo);
        }
    }
}
*/

void AudioPreviewEditor::PlaySound()
{
    //If sound asset is not ready yet no need to play it.
    if (asset.expired())
        return;

    if (!soundChannel)
    {
        soundChannel = framework_->Audio()->PlaySound(asset.lock(), SoundChannel::Ambient);
        playButton_->setText(tr("Stop"));

        AudioSignalLabel *audioSignalLabel = mainWidget_->findChild<AudioSignalLabel *>("audioSignalLabel");
        if (audioSignalLabel)
        {
            float duration = audioSignalLabel->GetAudioDuration();
            if (!playTimer_)
            {
                playTimer_ = new QTimer(this);
                playTimer_->setSingleShot(true);
                connect(playTimer_, SIGNAL(timeout()), this, SLOT(TimerTimeout()));
            }
            playTimer_->start(duration * 1000);
        }
    }
    else
    {
        //User pressed stop audio before audio clip was finnished.
        framework_->Audio()->Stop(soundChannel);
        soundChannel.reset();
        playButton_->setText(tr("Play"));
        if (playTimer_ && playTimer_->isActive())
            playTimer_->stop();
    }
}

void AudioPreviewEditor::TimerTimeout()
{
    if (asset.expired() || !soundChannel)
        return;

    framework_->Audio()->Stop(soundChannel);
    soundChannel.reset();
    playButton_->setText(tr("Play"));
}

void AudioPreviewEditor::OnAssetTransferSucceeded(AssetPtr asset)
{
    //Open();
}

void AudioPreviewEditor::OnAssetTransferFailed(IAssetTransfer *transfer, QString reason)
{
    LogError("AudioPreviewEditor::OnAssetTransferFailed: " + reason);
    //setText("Could not load asset: " + reason);
}


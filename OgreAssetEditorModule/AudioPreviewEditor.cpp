// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "AudioPreviewEditor.h"
#include "AudioSignalLabel.h"
#include "OgreAssetEditorModule.h"

#include "Framework.h"
#include "Application.h"
#include "UiProxyWidget.h"

#include "AudioAPI.h"

#include <QUiLoader>
#include <QFile>
#include <QLayout>
#include <QPushButton>
#include <QImage>
#include <QTimer>

#include "MemoryLeakCheck.h"

AudioPreviewEditor::AudioPreviewEditor(Framework *framework, const QString &name, QWidget *parent):
    QWidget(parent),
    framework_(framework),
    okButton_(0),
    playButton_(0),
    playTimer_(0)
{
    setObjectName(name);
    // Get ui service and create canvas
    ///\todo Use UiAPI
/*
    UiServiceInterface *ui= framework_->Get Service<UiServiceInterface>();
    if (!ui)
        return;

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

    okButton_ = mainWidget_->findChild<QPushButton *>("okButton");
    QObject::connect(okButton_, SIGNAL(clicked()), this, SLOT(Closed()));

    playButton_ = mainWidget_->findChild<QPushButton *>("playButton");
    QObject::connect(playButton_, SIGNAL(clicked()), this, SLOT(PlaySound()));

    // Add widget to UI via ui services module
    setWindowTitle(tr("Audio: ") + objectName());
    UiProxyWidget *proxy = ui->AddWidgetToScene(this);
    connect(proxy, SIGNAL(Closed()), this, SLOT(Closed()));
    proxy->show();
    ui->BringWidgetToFront(proxy);
*/
}

AudioPreviewEditor::~AudioPreviewEditor()
{
}

    ///\todo Regression. Reimplement using the new Asset API. -jj.
    /*
void AudioPreviewEditor::HandleAssetReady(AssetInterfacePtr asset)
{
    Service ManagerPtr service_manager = framework_->Get ServiceManager();
    if(service_manager)
    {
        if(service_manager->IsRegistered(Service::ST_Sound))
        {
            boost::shared_ptr<ISoundService> sound_service = 
                service_manager->G etS ervice<ISoundService>(Service::ST_Sound).lock();
            if(!sound_service)
                return;

            request_tag_ = sound_service->RequestSoundResource(QString::fromStdString(asset->GetId()));
        }
    }
}
    */

    ///\todo Regression. Reimplement using the new Asset API. -jj.
    /*
void AudioPreviewEditor::HandleResouceReady(Resource::Events::ResourceReady *res) 
{
    if(request_tag_ == res->tag_)
    {
        SoundResource *sound = dynamic_cast<SoundResource *>(res->resource_.get());
        if(sound)
        {
            QLabel *audioInfoLabel = findChild<QLabel*>("descriptionLabel");
            ISoundService::SoundBuffer buffer = sound->GetBuffer();
            audioInfoLabel->setText(QString(tr("Frequency: %1Hz")).arg(buffer.frequency_));
            
            int bits = 16;
            if(!buffer.sixteenbit_)
                bits = 8;
            audioInfoLabel->setText(audioInfoLabel->text() + QString(tr(" Bits: %1 bit")).arg(bits));

            QString stereo = "Stereo";
            if(!buffer.stereo_)
                stereo = "Mono";
            audioInfoLabel->setText(audioInfoLabel->text() + tr(" Format: ") + stereo);

            float duration;
            if(!buffer.stereo_)
                duration = float(buffer.data_.size() / ((bits / 8))) / float(buffer.frequency_);
            else
                duration = float(buffer.data_.size() / ((bits / 8) * 2)) / float(buffer.frequency_);
            audioInfoLabel->setText(audioInfoLabel->text() + QString(tr("\nDuration: %1 sec")).arg(duration));

            QVBoxLayout *layout = mainWidget_->findChild<QVBoxLayout*>("verticalLayout_2");
            if(layout)
            {
                AudioSignalLabel *audioSignalLabel = mainWidget_->findChild<AudioSignalLabel *>("audioSignalLabel");
                if(!audioSignalLabel)
                {
                    audioSignalLabel = new AudioSignalLabel(this);
                    audioSignalLabel->setObjectName("audioSignalLabel");
                    layout->addWidget(audioSignalLabel);
                }
                audioSignalLabel->SetAudioData(buffer.data_, buffer.frequency_, bits, buffer.stereo_);
            }
            assetId_ = QString(sound->GetId().c_str());
        }
    }
}
    */

void AudioPreviewEditor::Closed()
{
//    emit Closed(inventoryId_, assetType_);
}

void AudioPreviewEditor::PlaySound()
{/* ///\todo Regression. Reimplement. -jj.
    //If sound asset is not ready yet no need to play it.
    if(assetId_.size() <= 0)
        return;

    Service ManagerPtr service_manager = framework_->Get ServiceManager();
    if(service_manager)
    {
        if(service_manager->IsRegistered(Service::ST_Sound))
        {
            boost::shared_ptr<ISoundService> sound_service = 
                service_manager->G et Service<ISoundService>(Service::ST_Sound).lock();
            if(!sound_service)
                return;

            if(soundId_ == 0)
            {
                soundId_ = sound_service->PlaySound(assetId_, ISoundService::Ambient);
                playButton_->setText(tr("Stop"));
                
                AudioSignalLabel *audioSignalLabel = mainWidget_->findChild<AudioSignalLabel *>("audioSignalLabel");
                if(audioSignalLabel)
                {
                    float duration = audioSignalLabel->GetAudioDuration();
                    if(!playTimer_)
                    {
                        playTimer_ = new QTimer(this);
                        playTimer_->setSingleShot(true);
                        QObject::connect(playTimer_, SIGNAL(timeout()), this, SLOT(TimerTimeout()));
                    }
                    playTimer_->start(duration * 1000);
                }
            }
            else
            {
                //User pressed stop audio before audio clip was finnished.
                sound_service->StopSound(soundId_);
                soundId_ = 0;
                playButton_->setText(tr("Play"));
                if(playTimer_)
                {
                    if(playTimer_->isActive())
                        playTimer_->stop();
                }
            }
        }
    }
    */
}

void AudioPreviewEditor::TimerTimeout()
{
///\todo Regression. Reimplement. -jj.
/*
    //If sound asset is not ready yet no need to play it.
    if(assetId_.size() <= 0 || soundId_ == 0)
        return;

    Service ManagerPtr service_manager = framework_->Get ServiceManager();
    if(service_manager)
    {
        if(service_manager->IsRegistered(Service::ST_Sound))
        {
            boost::shared_ptr<ISoundService> sound_service = 
                service_manager->G et Service<ISoundService>(Service::ST_Sound).lock();
            if(!sound_service)
                return;

            sound_service->StopSound(soundId_);
            soundId_ = 0;
            playButton_->setText(tr("Play"));
        }
    }
*/
}

void AudioPreviewEditor::resizeEvent(QResizeEvent *ev)
{
    QWidget::resizeEvent(ev);
    emit WidgetResized(ev->size());
}

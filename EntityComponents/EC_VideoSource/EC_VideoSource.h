// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_VideoSource_EC_VideoSource_h
#define incl_EC_VideoSource_EC_VideoSource_h

#include "IComponent.h"
#include "IAttribute.h"
#include "Declare_EC.h"
#include "EC_3DCanvas.h"

#include <QWidget>
#include <QString>
#include <QTimer>
#include <QLabel>
#include <QSize>

#include <Phonon/VideoPlayer>
#include <Phonon/MediaObject>
#include <Phonon/VideoWidget>
#include <Phonon/MediaSource>
#include <Phonon/AudioOutput>

class EC_VideoSource : public IComponent
{
    DECLARE_EC(EC_VideoSource);
    Q_OBJECT
public:
    /// playback state enumeration
    enum PlaybackState
    {
        PS_Play,
        PS_Stop,
        PS_Pause
    };

    ~EC_VideoSource();
    virtual bool IsSerializable() const { return true; }

    Q_PROPERTY(QString videoSourceUrl READ getvideoSourceUrl WRITE setvideoSourceUrl);
    DEFINE_QPROPERTY_ATTRIBUTE(QString, videoSourceUrl);

    Q_PROPERTY(int submeshIndex READ getsubmeshIndex WRITE setsubmeshIndex);
    DEFINE_QPROPERTY_ATTRIBUTE(int, submeshIndex);

    Q_PROPERTY(int playbackState READ getplaybackState WRITE setplaybackState);
    DEFINE_QPROPERTY_ATTRIBUTE(int, playbackState);

    Q_PROPERTY(int refreshRate READ getrefreshRate WRITE setrefreshRate);
    DEFINE_QPROPERTY_ATTRIBUTE(int, refreshRate);

    Q_PROPERTY(float audioPlaybackVolume READ getaudioPlaybackVolume WRITE setaudioPlaybackVolume);
    DEFINE_QPROPERTY_ATTRIBUTE(float, audioPlaybackVolume);

    Q_PROPERTY(bool scaleDown READ getscaleDown WRITE setscaleDown);
    DEFINE_QPROPERTY_ATTRIBUTE(bool, scaleDown);

    Q_PROPERTY(bool looping READ getlooping WRITE setlooping);
    DEFINE_QPROPERTY_ATTRIBUTE(bool, looping);

public slots:
    void Play();
    void Stop();
    void Pause();
    /// Get each attribute value and send them over to sound service.
    void PlaybackFinished();

    virtual bool HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData* data);

private slots:
    void InitializePhonon();
    void UpdateSignals();
    void OnAttributeUpdated(IAttribute *attribute);
    /// Registers the action this EC provides to the parent entity, when it's set.
    void RegisterActions();
    void UpdateCanvas();

    void PlayerStateChanged(Phonon::State new_state, Phonon::State old_state);
    void BufferStatus(int filled);
    void StartVideoPlayback(bool has_video);

    ///\todo Regression. Reimplement using the new Asset API. -jj.
//    void LoadVideo(Foundation::AssetInterfacePtr asset);
    void LoadCurrentVideo();

private:
    explicit EC_VideoSource(IModule *module);

    EC_3DCanvas *Get3DCanvas();

    Phonon::VideoPlayer *player_;
    Phonon::VideoWidget *video_widget_;
    Phonon::MediaObject *media_object_;
    Phonon::AudioOutput *audio_output_;

    QString video_source_;
    EC_3DCanvas *canvas_;
    bool startup_checker_;

    bool start_canvas_;
    bool stop_canvas_;
    bool playing_canvas_;
    bool expecting_resources_;

    QSize original_size_;

    QString current_video_path_;
    QTimer *ready_poller_;
    QLabel *error_label_;

    event_category_id_t asset_event_category_;
};

#endif

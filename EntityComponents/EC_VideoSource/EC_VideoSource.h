// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_VideoSource_EC_VideoSource_h
#define incl_EC_VideoSource_EC_VideoSource_h

#include "IComponent.h"
#include "IAttribute.h"
#include "Declare_EC.h"
#include "EC_3DCanvas.h"
#include <QWidget>
#include <QString>
#include <Phonon/VideoPlayer>
#include <Phonon/MediaObject>
#include <Phonon/VideoWidget>


class EC_VideoSource : public IComponent
{
    DECLARE_EC(EC_VideoSource);
    Q_OBJECT
public:
    //! playback state enumeration
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

    Q_PROPERTY(bool triggerVideo READ gettriggerVideo WRITE settriggerVideo);
    DEFINE_QPROPERTY_ATTRIBUTE(bool, triggerVideo);

    Q_PROPERTY(float audioPlaybackVolume READ getaudioPlaybackVolume WRITE setaudioPlaybackVolume);
    DEFINE_QPROPERTY_ATTRIBUTE(float, audioPlaybackVolume);

    Q_PROPERTY(bool looping READ getlooping WRITE setlooping);
    DEFINE_QPROPERTY_ATTRIBUTE(bool, looping);

public slots:
    void Play();
    void Stop();
    void Pause();
    //! Get each attribute value and send them over to sound service.
    void PlaybackFinished();

private slots:
    void UpdateSignals();
    void AttributeUpdated(IAttribute *attribute);
    /// Registers the action this EC provides to the parent entity, when it's set.
    void RegisterActions();
    void UpdateCanvas();

private:
    explicit EC_VideoSource(IModule *module);

    Phonon::VideoPlayer *player_;
    QString video_source_;
    EC_3DCanvas *canvas_;
    bool startup_checker_;
};

#endif

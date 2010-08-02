// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_PhononPlayer_VideoPlayer_h
#define incl_PhononPlayer_VideoPlayer_h

#include <QWidget>
#include <QBoxLayout>
#include <phonon>

namespace Phonon
{
    class VideoPlayer;
    class MediaObject;
    class VideoWidget;
}

namespace PlayerService
{
    /// Render video content in a endless loop.
    class VideoPlayer : public QWidget
    {
        Q_OBJECT
    public:
        VideoPlayer(const QString &url);
        virtual ~VideoPlayer();
    private:
        Phonon::MediaObject* media_object_;
        Phonon::VideoWidget* video_widget_;
    public slots:
        void Play(const QString &url);
    private slots:
//        void Restart();
        void CheckState(Phonon::State new_state, Phonon::State old_state);
        void StartVideoPlayback(bool has_video);
        void RestartVideoPlayback();
    private:
        bool error_handled_;
        QBoxLayout *layout_;
    };
} // PlayerService

#endif // incl_PhononPlayer_VideoPlayer_h

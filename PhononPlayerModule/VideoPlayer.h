// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_PhononPlayer_VideoPlayer_h
#define incl_PhononPlayer_VideoPlayer_h

#include <QWidget>

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
        void Restart();
        void CheckState();
    };
} // PlayerService

#endif // incl_PhononPlayer_VideoPlayer_h

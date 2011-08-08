// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include "VlcFwd.h"
#include "PlayerStatus.h"

#include <QObject>
#include <QPixmap>
#include <QMutex>
#include <QImage>
#include <QTimer>
#include <QFrame>
#include <QList>
#include <QSize>
#include <QRect>
#include <QByteArray>

// Do not change the order of these includes. On windows we need 
// libvlc_structures.h to be included first before libvlc.h due to the proper stdint.h missing.
#include "vlc/libvlc_structures.h"
#include "vlc/libvlc.h"
#include "vlc/libvlc_media.h"
#include "vlc/libvlc_media_player.h"
#include "vlc/libvlc_events.h"

class VlcVideoWidget : public QFrame 
{

Q_OBJECT

public:
    /// Constructor
	VlcVideoWidget(const QList<QByteArray> &args);

    /// Deconstructor
	~VlcVideoWidget();

public slots:
	/// Open a source. Returns true on success, false on failed.
	/*! [file://]filename              Plain media file
	    http://ip:port/file            HTTP URL
	    ftp://ip:port/file             FTP URL
	    mms://ip:port/file             MMS URL
	    screen://                      Screen capture
	    [dvd://][device][@raw_device]  DVD device
	    [vcd://][device]               VCD device
	    [cdda://][device]              Audio CD device
	    udp:[[<source address>]@[<bind address>][:<bind port>]]
	*/
	bool OpenSource(const QString& videoUrl);

    /// Start playback if in stopped state, toggle pause otherwise.
    /// @return bool True if playing, false if paused.
    bool TogglePlay();

	/// Start playback
	void Play();

	/// Pause playback
	void Pause();

	/// Stop playback and rewind to beginning
	void Stop();

    /// Seek current media to time
    void Seek(boost::uint_least64_t time);

    /// Shutdown this widget and the vlc related instances
    void ShutDown();

    /// Return if initialized and ready for playback
    bool Initialized();

    /// Force to emit the idle image.
    void ForceIdleImage();

protected:
	/// Internal impl for providing memory
	void* InternalLock(void** pixelPlane);
	
    /// Internal impl for releasing memory
    void InternalUnlock(void* picture, void*const *pixelPlane);
	
    /// Internal impl for rendering
    void InternalRender(void* picture);

    /// Vlc callback for providing memory
    static void* CallBackLock(void* widget, void** pixelPlane);

    /// Vlc callback for releasing memory
	static void CallBackUnlock(void* widget, void* picture, void*const *pixelPlane);

    /// Vlc callback for rendering
	static void CallBackDisplay(void* widget, void* picture);

    static void VlcEventHandler(const libvlc_event_t *event, void *widget);

	/// QWidget override
	virtual void paintEvent(QPaintEvent *e);

    // Player status
    PlayerStatus status;
    QMutex statusAccess;

signals:
    /// Status update, see status.change for the type.
    void StatusUpdate(const PlayerStatus status);

    /// Rendering frame update.
    void FrameUpdate(QImage frame);

private slots:
    /// Determine video size. This is needed ad vlc cant give us video size before we start playing it.
    /// Hence we cant set the correct output size for the video. When size is determined we restart playback for proper rendering.
    void DetermineVideoSize();

    /// Kills the previous player and restarts playback with proper rendering.
    void RestartPlayback();

    /// Slot for timer to hit. This function inspects changes in the status and acts on them.
    /// Also emits StatusUpdate signal when needed.
    void StatusPoller();
    
private:
    /// Vlc main instance
	libvlc_instance_t *vlcInstance_;

    /// Vlc main player
	libvlc_media_player_t *vlcPlayer_;

    /// Vlc media
    libvlc_media_t* vlcMedia_;

	/// Caches the video frame for widget painting.
	QPixmap onScreenPixmap_;

    /// Pause pixmap for pretty rendering on pause state
    QPixmap pausePixmap_;

    /// Buffer used by VLC to draw into.
	QImage renderPixmap_;

    /// RGBA buffer.
    QImage rgbaBuffer_;

    /// This gets rendered when in stopped state or there is no video.
    QImage idleLogo_;

    /// This gets rendererd when there is no video output in the source media.
    QImage audioLogo_;

	/// Mutex used to protect access to onScreenPixmap_
	QMutex onScreenPixmapMutex_;
	
    /// Mutex used to protect access to renderPixmap_
	QMutex renderPixmapMutex_;

    /// Main Tundra Vlc plugin
    VlcPlugin *vlcPlugin_;

    /// Status poll timer
    QTimer pollTimer_;

    /// Own boolean to determine if this is a audio only source.
    /// Getting this information from libvlc is surprisingly hard, so we do it on the fly.
    bool hasVideoOut_;

};

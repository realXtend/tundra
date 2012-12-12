// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "VlcFwd.h"
#include "SceneFwd.h"
#include "AssetFwd.h"
#include "AssetRefListener.h"

#include "IComponent.h"
#include "IAttribute.h"
#include "AssetReference.h"

#include <QImage>
#include <QTimer>
#include <QMenu>
#include <QAbstractAnimation>

class EC_Mesh;
class EC_WidgetCanvas;
class RaycastResult;

/// Shows various media formats supported by VLC on 3D object.
/** Depends on EC_Mesh and EC_WidgetCanvas. */
class EC_MediaPlayer : public IComponent
{
    Q_OBJECT

public:
    /// Constructor.
    explicit EC_MediaPlayer(Scene* scene);

    /// Destructor.
    virtual ~EC_MediaPlayer();

    /*! Media source reference. Accepted formats are:
        [file://]filename              Plain media file
        http://ip:port/file            HTTP URL
        ftp://ip:port/file             FTP URL
        mms://ip:port/file             MMS URL
        screen://                      Screen capture
        [dvd://][device][@raw_device]  DVD device
        [vcd://][device]               VCD device
        [cdda://][device]              Audio CD device
        udp:[[<source address>]@[<bind address>][:<bind port>]]
    */
    Q_PROPERTY(AssetReference sourceRef READ getsourceRef WRITE setsourceRef);
    DEFINE_QPROPERTY_ATTRIBUTE(AssetReference, sourceRef);

    /// Rendering target submesh index.
    Q_PROPERTY(int renderSubmeshIndex READ getrenderSubmeshIndex WRITE setrenderSubmeshIndex);
    DEFINE_QPROPERTY_ATTRIBUTE(int, renderSubmeshIndex);

    /// If rendering to the target submesh index is enabled
    Q_PROPERTY(bool enabled READ getenabled WRITE setenabled);
    DEFINE_QPROPERTY_ATTRIBUTE(bool, enabled);

    /// Target material illumination, can be used to see video in low light/shadow conditions.
    Q_PROPERTY(bool illuminating READ getilluminating WRITE setilluminating);
    DEFINE_QPROPERTY_ATTRIBUTE(bool, illuminating);
    
    /// If component is interactive, it will show a context menu when left clicked.
    /// If you want finer control how and where to show the menu, use GetContextMenu().
    Q_PROPERTY(bool interactive READ getinteractive WRITE setinteractive);
    DEFINE_QPROPERTY_ATTRIBUTE(bool, interactive);

    /// If we should let VLC stream or download the whole file before allowing playback.
    /// Setting this to false has the benefit of using our asset cache and you don't have to buffer during playback.
    /// @note The media will only be directly downloaded from http/https sources, other wise passed to VLC to handle streaming.
    Q_PROPERTY(bool streamingAllowed READ getstreamingAllowed WRITE setstreamingAllowed);
    DEFINE_QPROPERTY_ATTRIBUTE(bool, streamingAllowed);
    
    COMPONENT_NAME("EC_MediaPlayer", 37)

public slots:
    /// Starts playing the video if stopped or paused.
    void Play();

    /// Pauses the video if playing.
    void Pause();

    /// Plays video is stopped or paused, pauses otherwise.
    void PlayPauseToggle();

    /// Stop video playback. This will reset the timer to the start of the video.
    void Stop();

    /// Seeks media to desired seconds. This will be done only if media is seekable.
    /// Will seek only if media is playing, as long as one has been loaded successfully.
    /// @return bool True if succesfull, false other wise.
    bool SeekMedia(float timeInSeconds);

    /// Returns current video state. QAbstractAnimation::State can be Running (2), Paused (1) and Stopped (0).
    /// This enum is used because its auto exposed to scripting etc. and suits our purposes here.
    QAbstractAnimation::State GetMediaState() const;

    /// Returns the current loaded media total lenght in seconds. See GetMediaTime() for the current playback time.
    /// @return float Media lenght in seconds.
    /// @note If the media lenght cannot be resolved or no media is loaded returns 0.0.
    float GetMediaLenght();

    /// Returns the current media playback time in seconds. See GetMediaLenght() for the total lenght of the media.
    /// @return float Media time in seconds.
    /// @note Will return time also if paused/stopped. If the media time cannot be resolved or no media is loaded returns 0.0.
    float GetMediaTime();

    /// Returns if the media asset is being downloaded at this time.
    /// @note Will return false always if streamingAllowed is set to false.
    bool IsDownloadingMedia() { return pendingMediaDownload_; }

    /// Show/hide the player widget.
    void ShowPlayer(bool visible = true);

    /// Get the context menu. This can be handy if you don't want to have interactive attribute on,
    /// but still want to show the context menu in your own code.
    /// @note The QMenu will destroy itself when closed, you don't need to free the ptr.
    QMenu *GetContextMenu();

signals:
    /// This signal is emitted once the current media asset has been downloaded and is ready for playback.
    /// @param bool If download was succesfull true, false otherwise.
    /// @param QString Media source ref. This is the same as sourceRef.ref but provided here for convenience. 
    void MediaDownloaded(bool succesfull, QString source);

private slots:
    /// Callback to render content to the 3D target.
    void OnFrameUpdate(QImage frame); 

    /// Handler for window resize signal.
    void RenderWindowResized();

    /// Prepares everything related to the parent entity and other needed components.
    void PrepareComponent();

    /// Handler when EC_Mesh emits that the mesh is ready.
    void TargetMeshReady();

    /// Handler when EC_Mesh emits that a material has changed on one of its sub meshes.
    /// We inspect if the index is same as we are rendering to. If this is detected we re-apply our material to the sub mesh.
    void TargetMeshMaterialChanged(uint index, const QString &material);

    /// Resets the submesh index to 0.
    void ResetSubmeshIndex();

    /// Monitors this entitys added components.
    void ComponentAdded(IComponent *component, AttributeChange::Type change);

    /// Monitors this entitys removed components.
    void ComponentRemoved(IComponent *component, AttributeChange::Type change);

    /// Get parent entitys EC_Mesh. Return 0 if not present.
    EC_Mesh *GetMeshComponent();

    /// Get parent entitys EC_WidgetCanvas. Return 0 if not present.
    EC_WidgetCanvas *GetSceneCanvasComponent();

    /// Monitors entity mouse clicks.
    void EntityClicked(Entity *entity, Qt::MouseButton button, RaycastResult *raycastResult);

    /// Callback for mediaDownloader_
    void OnMediaLoaded(AssetPtr asset);

    /// Callback for mediaDownloader_
    void OnMediaFailed(IAssetTransfer *transfer, QString reason);

private:
    /// Monitors this components Attribute changes.
    void AttributesChanged();

    /// Vlc media player widget.
    VlcMediaPlayer *mediaPlayer_;

    /// Boolean for tracking if this component has been prepared properly.
    /** Guarantees: 
        - EC_Mesh is present and loaded to Ogre, ready for rendering.
        - EC_WidgetCanvas is present.
        - Parent Entity is valid and set.
        Does not guarantee:
        - EC_Mesh having submesh index of our 'renderSubmeshIndex' Attribute
        - EC_Mesh being visible. 
    */
    bool componentPrepared_;

    /// The scene canvas that we are using. This needs to be unique to enable
    /// multiple media players in a entity (rendering to different submesh indexes). 
    /// This is used to perform cleanup when this component is destroyed.
    QString sceneCanvasName_;

    /// Internal time for updating the rendering after a window resize event.
    QTimer *resizeRenderTimer_;

    /// Helper for manual downloads via asset api, this will be used if attribute 'streamingAllowed' is false.
    AssetRefListener *mediaDownloader_;

    /// Download indicator logo to inworld object.
    QImage downloadingLogo_;

    /// Track if we have a pending download operation.
    bool pendingMediaDownload_;
};
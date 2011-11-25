// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include "IModule.h"
#include <QObject>

#include "ui_AssetInterestSettings.h"

#include "SceneFwd.h"
#include "AssetFwd.h"
#include "OgreModuleFwd.h"
#include "Math/MathFwd.h"

#include <OgreTextureManager.h>
#include <OgreMaterialManager.h>
#include <OgreMeshManager.h>
#include <OgreResourceBackgroundQueue.h>

#include <QString>
#include <QSet>
#include <QTimer>
#include <QPointer>

class EC_Camera;
class Entity;

/** AssetInterestPlugin monitors scene asset references and cameras.

    This plugin can be configured from scripts in the scene to behave depending
    on how heavy the scene is. This plugin only implements unloading assets
    from AssetAPI after the fact that they have already been loaded. This has the downside
    of still downloading and loading everything in a big scene. We can only immediately 
    unload all that are outside of our interest and keep it that way until they are
    interesting to us again. 
    
    Until we have a better mechanism in the core sync manager to not send Entities that
    are outside of our interest (there for we never get the asset references) this
    plugin is one way to implement something quickly and to prototype. This plugin
    does not try to be a end-all-be-all solution for the client side scalability problem!
*/
class AssetInterestPlugin : public IModule
{

Q_OBJECT

/// Interest radius from the active EC_Camera. Default value is 100.0.
/// Having this <= 0 equals to setting 'enabled' to false.
Q_PROPERTY(double interestRadius READ InterestRadius WRITE SetInterestRadius)

/// Enable tracking of active EC_Camera. Default value is false as in this plugin wont do anything! 
/// Enable functionality from scripts or built in UI from settings menu.
Q_PROPERTY(bool enabled READ Enabled WRITE SetEnabled)

/// Tells if we should unload/load textures (of materials), this indirectly
/// also means that materials that use the textures will be unloaded. Default value is true.
Q_PROPERTY(bool processTextures READ IsProcessingTextures WRITE SetProcessTextures)

/// Tells if we should unload/load meshes. Default value is false.
/// @note Even if this if enabled you will still see gray meshes in the scene. 
/// @todo Make meshes actually hide from rendering when they are unloaded.
Q_PROPERTY(bool processMeshes READ IsProcessingMeshes WRITE SetProcessMeshes)

/// Time in milliseconds we should wait between asset loads. Minimum is 1. Default value is 100.
Q_PROPERTY(int waitAfterLoad READ WaitAfterLoad WRITE SetWaitAfterLoad)

/// Debug draw radius. Can be useful when finding good radius limits for your scene. Default value is false.
Q_PROPERTY(bool drawDebug READ DragDebug WRITE SetDrawDebug)

public:
    AssetInterestPlugin();
    virtual ~AssetInterestPlugin();

    void Initialize();
    void Unload();
    void Update(f64 frametime);

    double interestRadius;
    int waitAfterLoad;

    bool enabled;
    bool processTextures;
    bool processMeshes;  
    bool drawDebug;

public slots:
    bool Enabled()                              { return enabled; }
    bool IsProcessingTextures()                 { return processTextures; }
    bool IsProcessingMeshes()                   { return processMeshes; }
    bool DragDebug()                            { return drawDebug; }
    double InterestRadius()                     { return interestRadius; }
    int WaitAfterLoad()                         { return waitAfterLoad; }

    void SetEnabled(bool enabled_);
    void SetProcessTextures(bool process);
    void SetProcessMeshes(bool process);
    void SetDrawDebug(bool draw);
    void SetInterestRadius(double radius);
    void SetWaitAfterLoad(int intervalMsec);

    void UiToggleSettings();

private slots:
    void TransferDone()                         { if (!loadWaitTimer_.isActive()) loadWaitTimer_.start(waitAfterLoad); }
    void TransferTimeout()                      { shouldLoad_ = true; }

    void ReadConfig();
    void WriteConfig();

    void UiSetEnabled(bool enabled);

private:
    void LoadEverythingBack();

    Ogre::TextureManager &TexMan()              { return Ogre::TextureManager::getSingleton(); }
    Ogre::MaterialManager &MatMan()             { return Ogre::MaterialManager::getSingleton(); }
    Ogre::MeshManager &MeshMan()                { return Ogre::MeshManager::getSingleton(); }
    Ogre::ResourceBackgroundQueue &ResMan()     { return Ogre::ResourceBackgroundQueue::getSingleton(); }

    /// Returns if this reference should be processed by us.
    bool ShouldProcess(const QString &assetRef);

    /// Return if we are connected to a server.
    bool Connected();

    /// Get scenes main camera parent entity.
    Entity *MainCamera();

    /// Boolean for tracking if we should load
    bool shouldLoad_;

    /// Current process time.
    float processTick_;

    /// Timer to do the load wait delay.
    QTimer loadWaitTimer_;

    // Lists for handling asset references
    QSet<QString> matsPendingLoad_;
    QSet<QString> matsPendingUnload_;
    QSet<QString> meshPendingLoad_;
    QSet<QString> meshPendingUnload_;

    // User interface
    QPointer<QWidget> widget_;
    Ui::AssetInterestSettings ui_;
};

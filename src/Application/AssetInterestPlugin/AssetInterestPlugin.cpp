// For conditions of distribution and use, see copyright notice in license.txt

#include "AssetInterestPlugin.h"

#include "Framework.h"
#include "Profiler.h"
#include "CoreDefines.h"
#include "Application.h"
#include "LoggingFunctions.h"
#include "Math/float3.h"
#include "Geometry/Circle.h"

#include "SceneAPI.h"
#include "Scene.h"
#include "Entity.h"

#include "AssetAPI.h"
#include "IAsset.h"
#include "IAssetTransfer.h"

#include "TundraLogicModule.h"
#include "Client.h"
#include "Server.h"

#include "OgreRenderingModule.h"
#include "Renderer.h"
#include "OgreMeshAsset.h"
#include "OgreMaterialAsset.h"
#include "OgreWorld.h"
#include "TextureAsset.h"

#include "EC_Placeable.h"
#include "EC_Camera.h"
#include "EC_Mesh.h"
#include "EC_Material.h"

#include "UiAPI.h"
#include "UiMainWindow.h"

#include <Ogre.h>
#include <OgreResource.h>
#include <OgreMaterial.h>
#include <OgreTexture.h>
#include <OgreMesh.h>
#include <OgreEntity.h>

#include <kNet/Network.h>

#include <QMenuBar>
#include <QMenu>
#include <QAction>

AssetInterestPlugin::AssetInterestPlugin() :
    IModule("AssetInterestPlugin"),
    // Internal variables
    processTick_(0.0f),
    shouldLoad_(true),
    widget_(0),
    // QObject properties
    interestRadius(100.0f),
    enabled(false),
    processTextures(true),
    processMeshes(false),
    inspectRemovedEntities(false),
    drawDebug(false),
    waitAfterLoad(100)
{
    loadWaitTimer_.setSingleShot(true);
    connect(&loadWaitTimer_, SIGNAL(timeout()), SLOT(TransferTimeout()));
}

AssetInterestPlugin::~AssetInterestPlugin()
{
}

void AssetInterestPlugin::Initialize()
{
    if (framework_->IsHeadless())
        return;

    framework_->RegisterDynamicObject("assetinterest", this);

    QMenuBar *menuBar = framework_->Ui()->MainWindow()->menuBar();
    if (menuBar)
    {
        QMenu *settingsMenu = framework_->Ui()->MainWindow()->menuBar()->findChild<QMenu*>("SettingsMenu");
        if (!settingsMenu)
        {
            settingsMenu = menuBar->addMenu("&Settings");
            settingsMenu->setObjectName("SettingsMenu");
        }
        QAction *toggleAct = settingsMenu->addAction("Asset Interest");
        if (toggleAct)
        {
            widget_ = new QWidget(framework_->Ui()->MainWindow(), Qt::Tool);
            ui_.setupUi(widget_);

            ui_.enabledCheckBox->setChecked(enabled);
            ui_.processTexturesCheckBox->setChecked(processTextures);
            ui_.processMeshesCheckBox->setChecked(processMeshes);
            ui_.drawDebugCheckBox->setChecked(drawDebug);
            ui_.loadIntervalSpinBox->setValue(waitAfterLoad);
            ui_.radiusDoubleSpinBox->setValue(interestRadius);

            UiSetEnabled(ui_.enabledCheckBox->isChecked());
                        
            connect(toggleAct, SIGNAL(triggered()), SLOT(UiToggleSettings()));
            connect(ui_.enabledCheckBox, SIGNAL(clicked(bool)), SLOT(SetEnabled(bool)));
            connect(ui_.processTexturesCheckBox, SIGNAL(clicked(bool)), SLOT(SetProcessTextures(bool)));
            connect(ui_.processMeshesCheckBox, SIGNAL(clicked(bool)), SLOT(SetProcessMeshes(bool)));
            connect(ui_.inspectRemovedEntitiesCheckBox, SIGNAL(clicked(bool)), SLOT(SetInspectRemovedEntities(bool)));
            connect(ui_.drawDebugCheckBox, SIGNAL(clicked(bool)), SLOT(SetDrawDebug(bool)));
            connect(ui_.loadIntervalSpinBox, SIGNAL(valueChanged(int)), this, SLOT(SetWaitAfterLoad(int)));
            connect(ui_.radiusDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(SetInterestRadius(double)));
            connect(ui_.buttonClose, SIGNAL(clicked()), widget_, SLOT(hide()));
        }
    }

    connect(framework_->Scene(), SIGNAL(SceneAdded(const QString&)), SLOT(OnSceneAdded(const QString&))); 
}

void AssetInterestPlugin::Unload()
{
    if (widget_)
        delete widget_;
}

template<typename T>
int CountSize(Ogre::MapIterator<T> iter)
{
    int count = 0;
    while(iter.hasMoreElements())
    {
        ++count;
        iter.getNext();
    }
    return count;
}

void AssetInterestPlugin::Update(f64 frametime)
{
    // Initial checks
    if (framework_->IsHeadless())
        return;
    if (!Connected())
        return;

    // If we are in disabled mode only return if there is no pending load/unloads
    if (!enabled || interestRadius <= 0)
    {
        if (matsPendingLoad_.isEmpty() && matsPendingUnload_.isEmpty() && 
            meshPendingLoad_.isEmpty() && meshPendingUnload_.isEmpty())
            return;
    }
    // To draw debug we need to be enabled and have a interest radius
    else if (drawDebug)
    {
        Entity *d_cameraEnt = MainCamera();
        if (d_cameraEnt && d_cameraEnt->ParentScene())
        {
            EC_Placeable *d_placeable = dynamic_cast<EC_Placeable*>(d_cameraEnt->GetComponent(EC_Placeable::TypeNameStatic()).get());
            OgreWorld *d_ogreWorld = d_cameraEnt->ParentScene()->GetWorld<OgreWorld>().get();
            if (d_placeable && d_ogreWorld)
                d_ogreWorld->DebugDrawSphere(d_placeable->WorldPosition(), interestRadius, 8748, 1, 0, 0, true);
        }
    }

    if (widget_ && widget_->isVisible())
    {
        if (ui_.meshManagerStatus)
            ui_.meshManagerStatus->setText("Count: " + QString::number(CountSize(MeshMan().getResourceIterator())) + 
                " Memory usage: " + kNet::FormatBytes((u64)MeshMan().getMemoryUsage()).c_str());
        if (ui_.textureManagerStatus)
            ui_.textureManagerStatus->setText("Count: " + QString::number(CountSize(TexMan().getResourceIterator())) + 
                " Memory usage: " + kNet::FormatBytes((u64)TexMan().getMemoryUsage()).c_str());
    }

    // We don't have to run this logic very often.
    processTick_ += frametime;
    if (processTick_ < 0.1)
        return;
    processTick_ = 0.0;

    // These lists will have all asset refs inside our radius.
    // With this we can skip unloading refs that are both 
    // in and outside of our interest radius!
    QSet<QString> matsInRadius;
    QSet<QString> meshesInRadius;

    // Iterate scene if radius based search is enabled
    if (enabled && interestRadius > 0.1f)
    {
        // Get needed data
        Entity *cameraEnt = MainCamera();
        if (!cameraEnt || !cameraEnt->ParentScene())
            return;
        EC_Camera *camera = dynamic_cast<EC_Camera*>(cameraEnt->GetComponent(EC_Camera::TypeNameStatic()).get());
        EC_Placeable *placeable = dynamic_cast<EC_Placeable*>(cameraEnt->GetComponent(EC_Placeable::TypeNameStatic()).get());
        if (!camera || !placeable)
            return;
        float3 cameraPos = placeable->WorldPosition();

        PROFILE(AssetInterestPlugin_Update_Iter_Scene);

        Scene *scene = cameraEnt->ParentScene();
        Scene::EntityMap::const_iterator iter = scene->begin();
        Scene::EntityMap::const_iterator end = scene->end();
        while (iter != end)
        {
            Entity *ent = iter->second.get();
            ++iter;
            if (!ent)
                continue;

            // We assume there is a single placeable and mesh for now
            EC_Placeable *entP = dynamic_cast<EC_Placeable*>(ent->GetComponent(EC_Placeable::TypeNameStatic()).get());
            EC_Mesh *entM = dynamic_cast<EC_Mesh*>(ent->GetComponent(EC_Mesh::TypeNameStatic()).get());
            if (!entP || !entM)
                continue;

            // Check if internal Ogre::Entity* has been created.
            if (entM->HasMesh())
            {
                // Distance from the camera.
                float distance = cameraPos.Distance(entP->WorldPosition());

                // Materials and textures
                if (processTextures)
                {   
                    AssetReferenceList mats = entM->getmeshMaterial();
                    for (int iMat=0; iMat<mats.Size(); iMat++)
                    {
                        QString ref = mats[iMat].ref;
                        if (!ShouldProcess(ref))
                            continue;

                        OgreMaterialAsset *matAsset = dynamic_cast<OgreMaterialAsset*>(GetFramework()->Asset()->GetAsset(ref).get());
                        if (!matAsset)
                            continue;

                        if (distance > interestRadius)
                        {
                            if (matAsset->IsLoaded() && !matsPendingUnload_.contains(ref))
                                matsPendingUnload_ << ref;
                        }
                        else
                        {
                            if (!matAsset->IsLoaded() && !matsPendingLoad_.contains(ref))
                                matsPendingLoad_ << ref;
                            if (!matsInRadius.contains(ref))
                                matsInRadius << ref;
                        }
                    }
                }

                // Meshes
                if (processMeshes)
                {
                    QString ref = entM->getmeshRef().ref;
                    if (ShouldProcess(ref))
                    {
                        OgreMeshAsset *meshAsset = dynamic_cast<OgreMeshAsset*>(GetFramework()->Asset()->GetAsset(ref).get());
                        if (meshAsset)
                        {
                            if (distance > interestRadius)
                            {
                                if (meshAsset->IsLoaded() && !meshPendingUnload_.contains(ref))
                                    meshPendingUnload_ << ref;
                            }
                            else
                            {
                                if (!meshAsset->IsLoaded() && !meshPendingLoad_.contains(ref))
                                    meshPendingLoad_ << ref;
                                if (!meshesInRadius.contains(ref))
                                    meshesInRadius << ref;
                            }
                        }
                    }
                }
            }
        }

        ELIFORP(AssetInterestPlugin_Update_Iter_Scene);
    }

    PROFILE(AssetInterestPlugin_Update_Unload);

    // Process next material unload. If our interest radius has 
    // materials that are marked for unload, do not unload them.
    matsPendingUnload_ -= matsInRadius;
    if (!matsPendingUnload_.isEmpty())
    {
        QSet<QString>::iterator unloadIter = matsPendingUnload_.begin();
        if (unloadIter != matsPendingUnload_.end())
        {
            QString matUnloadRef = *unloadIter;
            OgreMaterialAsset *matAsset = dynamic_cast<OgreMaterialAsset*>(GetFramework()->Asset()->GetAsset(matUnloadRef).get());
            if (matAsset)
            {
                matAsset->Unload();

                int texUnloaded = 0;
                std::vector<AssetReference> matDeps = matAsset->FindReferences();
                for(uint iDep=0; iDep<matDeps.size(); iDep++)
                {
                    QString depRef = matDeps[iDep].ref;
                    if (GetFramework()->Asset()->GetResourceTypeFromAssetRef(depRef) == "Texture")
                    {
                        TextureAsset *texAsset = dynamic_cast<TextureAsset*>(GetFramework()->Asset()->GetAsset(depRef).get());
                        if (texAsset && texAsset->IsLoaded())
                        {
                            texAsset->Unload();
                            if (texAsset->DiskSourceType() == IAsset::Programmatic)
                                framework_->Asset()->ForgetAsset(depRef, false);
                            texUnloaded++;
                        }
                    }
                }
                if (matAsset->DiskSourceType() == IAsset::Programmatic)
                    framework_->Asset()->ForgetAsset(matUnloadRef, false);

                if (widget_ && widget_->isVisible())
                    ui_.unloadMaterial->setText(QString::number(texUnloaded) + " textures + " + matUnloadRef);
            }
            matsPendingUnload_.erase(unloadIter);
        }
    }
    else if (widget_ && widget_->isVisible() && ui_.unloadMaterial->text() != "none")
        ui_.unloadMaterial->setText("none");

    // Process next mesh unload. If our interest radius has 
    // meshes that are marked for unload, do not unload them.
    meshPendingUnload_ -= meshesInRadius;
    if (!meshPendingUnload_.isEmpty())
    {
        QSet<QString>::iterator unloadIter = meshPendingUnload_.begin();
        if (unloadIter != meshPendingUnload_.end())
        {
            QString meshUnloadRef = *unloadIter;
            OgreMeshAsset *meshAsset = dynamic_cast<OgreMeshAsset*>(GetFramework()->Asset()->GetAsset(meshUnloadRef).get());
            if (meshAsset)
            {
                meshAsset->Unload();
                if (meshAsset->DiskSourceType() == IAsset::Programmatic)
                    framework_->Asset()->ForgetAsset(meshUnloadRef, false);

                if (widget_ && widget_->isVisible())
                    ui_.unloadMesh->setText(meshUnloadRef);
            }
        }
        meshPendingUnload_.erase(unloadIter);
    } 
    else if (widget_ && widget_->isVisible() && ui_.unloadMesh->text() != "none")
        ui_.unloadMesh->setText("none");

    ELIFORP(AssetInterestPlugin_Update_Unload);
    PROFILE(AssetInterestPlugin_Update_Reload);

    // @note Using RequestAsset() to reload also loads all dependencies (eg. textures for materials) 
    // with threading if supported on the platform/build. It will also utilize the asset cache.
    // It would be nice if we could force HttpAssetProvider to skip the header check 
    // (to eliminate all networking on this RequestAsset() call) but there is no 
    // such mechanism right now in Tundra, and no we cannot call asset->LoadFromCache()
    // because it does not allow asynchronous loading and we would start getting noticeable lag
    // on eg. big textures.

    // Process next material load
    if (shouldLoad_ && !matsPendingLoad_.isEmpty())
    {
        QSet<QString>::iterator loadIter = matsPendingLoad_.begin();
        if (loadIter != matsPendingLoad_.end())
        {
            QString matLoadRef = *loadIter;
            AssetTransferPtr tranfer = GetFramework()->Asset()->RequestAsset(matLoadRef);
            if (tranfer.get())
            {
                if (connect(tranfer.get(), SIGNAL(Succeeded(AssetPtr)), SLOT(TransferDone()), Qt::UniqueConnection) &&
                    connect(tranfer.get(), SIGNAL(Failed(IAssetTransfer*, QString)), SLOT(TransferDone()), Qt::UniqueConnection))
                {
                    shouldLoad_ = false;
                    if (widget_ && widget_->isVisible())
                        ui_.loadMaterial->setText("Textures of " + matLoadRef);
                }
            }
            matsPendingLoad_.erase(loadIter);
        }
    }
    else if (widget_ && widget_->isVisible() && shouldLoad_ && matsPendingLoad_.isEmpty() && ui_.loadMaterial->text() != "none")
        ui_.loadMaterial->setText("none");

    // Process next mesh load
    if (shouldLoad_ && !meshPendingLoad_.isEmpty())
    {
        QSet<QString>::iterator loadIter = meshPendingLoad_.begin();
        if (loadIter != meshPendingLoad_.end())
        {
            QString meshLoadRef = *loadIter;
            AssetTransferPtr tranfer = GetFramework()->Asset()->RequestAsset(meshLoadRef);
            if (tranfer.get())
            {
                if (connect(tranfer.get(), SIGNAL(Succeeded(AssetPtr)), SLOT(TransferDone()), Qt::UniqueConnection) &&
                    connect(tranfer.get(), SIGNAL(Failed(IAssetTransfer*, QString)), SLOT(TransferDone()), Qt::UniqueConnection))
                {
                    shouldLoad_ = false;
                    if (widget_ && widget_->isVisible())
                        ui_.loadMesh->setText(meshLoadRef);
                }
            }
            meshPendingLoad_.erase(loadIter);
        }
    } 
    else if (widget_ && widget_->isVisible() && shouldLoad_ && meshPendingLoad_.isEmpty() && ui_.loadMesh->text() != "none")
        ui_.loadMesh->setText("none");

    ELIFORP(AssetInterestPlugin_Update_Reload);
}

void AssetInterestPlugin::UiToggleSettings()
{
    if (widget_)
        widget_->setVisible(!widget_->isVisible());
}

void AssetInterestPlugin::UiSetEnabled(bool enabled)
{
    if (!widget_)
        return;

    ui_.processTexturesCheckBox->setEnabled(enabled);
    ui_.processMeshesCheckBox->setEnabled(enabled);
    ui_.drawDebugCheckBox->setEnabled(enabled);
    ui_.loadIntervalSpinBox->setEnabled(enabled);
    ui_.radiusDoubleSpinBox->setEnabled(enabled);
}

void AssetInterestPlugin::OnSceneAdded(const QString &name)
{
    ScenePtr scene = framework_->Scene()->GetScene(name);
    if (scene.get())
    {
        connect(scene.get(), SIGNAL(ComponentRemoved(Entity*, IComponent*, AttributeChange::Type)), 
            this, SLOT(OnComponentRemoved(Entity*, IComponent*, AttributeChange::Type)), Qt::UniqueConnection);
        connect(scene.get(), SIGNAL(EntityRemoved(Entity*, AttributeChange::Type)), 
            this, SLOT(OnEntityRemoved(Entity*, AttributeChange::Type)), Qt::UniqueConnection);
    }
}

void AssetInterestPlugin::OnEntityRemoved(Entity *entity, AttributeChange::Type change)
{
    if (inspectRemovedEntities && entity)
    {
        OnComponentRemoved(entity, entity->GetComponent<EC_Mesh>().get(), change);
        OnComponentRemoved(entity, entity->GetComponent<EC_Material>().get(), change);
    }
}

void AssetInterestPlugin::OnComponentRemoved(Entity *entity, IComponent *component, AttributeChange::Type change)
{
    if (!inspectRemovedEntities || !entity || !component)
        return;

    if (component->TypeName() == EC_Mesh::TypeNameStatic())
    {
        EC_Mesh *entM = dynamic_cast<EC_Mesh*>(component);
        if (entM && processTextures)
        {
            AssetReferenceList mats = entM->getmeshMaterial();
            for (int iMat=0; iMat<mats.Size(); iMat++)
            {
                QString ref = mats[iMat].ref;
                if (!ShouldProcess(ref, true))
                    continue;

                OgreMaterialAsset *matAsset = dynamic_cast<OgreMaterialAsset*>(GetFramework()->Asset()->GetAsset(ref).get());
                if (matAsset && matAsset->IsLoaded())
                    if (!matsPendingUnload_.contains(ref))
                        matsPendingUnload_ << ref;
            }
        }
        if (entM && processMeshes)
        {
            if (ShouldProcess(entM->getmeshRef().ref, true) && !meshPendingUnload_.contains(entM->getmeshRef().ref))
                meshPendingUnload_ << entM->getmeshRef().ref;
        }
    }
    else if (component->TypeName() == EC_Material::TypeNameStatic())
    {
        EC_Material *material = dynamic_cast<EC_Material*>(component);
        if (material && processTextures)
        {
            if (!material->getinputMat().isEmpty() && ShouldProcess(material->getinputMat(), true))
            {
                OgreMaterialAsset *inputMatAsset = dynamic_cast<OgreMaterialAsset*>(GetFramework()->Asset()->GetAsset(material->getinputMat()).get());
                if (inputMatAsset && inputMatAsset->IsLoaded())
                    if (!matsPendingUnload_.contains(material->getinputMat()))
                        matsPendingUnload_ << material->getinputMat();
            }
            if (!material->getoutputMat().isEmpty() && ShouldProcess(material->getoutputMat(), true))
            {
                OgreMaterialAsset *outPutMatAsset = dynamic_cast<OgreMaterialAsset*>(GetFramework()->Asset()->GetAsset(material->getoutputMat()).get());
                if (outPutMatAsset && outPutMatAsset->IsLoaded())
                    if (!matsPendingUnload_.contains(material->getoutputMat()))
                        matsPendingUnload_ << material->getoutputMat();
            }
        }
    }
}

bool AssetInterestPlugin::ShouldProcess(const QString &assetRef, bool acceptLocal)
{
    // At the moment we only want to handle external web refs.
    const QString trimmedRef = assetRef.trimmed();
    if (trimmedRef.isEmpty())
        return false;
    if (acceptLocal && trimmedRef.startsWith("local://"))
        return true;
    // Would be nice to use this but it is WAY too heavy to do frequently.
    //return (AssetAPI::ParseAssetRef(assetRef.trimmed()) == AssetAPI::AssetRefExternalUrl);
    if (trimmedRef.startsWith("http://") || trimmedRef.startsWith("https://"))
        return true;
    return false;
}

bool AssetInterestPlugin::Connected()
{
    TundraLogic::TundraLogicModule *tMod = GetFramework()->GetModule<TundraLogic::TundraLogicModule>();
    if (tMod)
        return tMod->IsServer() ? false : tMod->GetClient()->IsConnected();
    return false;
}

Entity *AssetInterestPlugin::MainCamera()
{
    OgreRenderer::OgreRenderingModule *orMod = GetFramework()->GetModule<OgreRenderer::OgreRenderingModule>();
    if (orMod && orMod->GetRenderer().get())
        return orMod->GetRenderer()->MainCamera();
    return 0;
}

void AssetInterestPlugin::SetEnabled(bool enabled_)
{
    enabled = enabled_;
    if (ui_.enabledCheckBox && ui_.enabledCheckBox->isChecked() != enabled)
        ui_.enabledCheckBox->setChecked(enabled);
    UiSetEnabled(enabled);

    // If we are now disabled we should restore all assets as loaded.
    // Otherwise parts of the scene will stay "disabled" aka gray.
    if (!enabled)
        LoadEverythingBack();
}

void AssetInterestPlugin::SetProcessTextures(bool process)
{
    processTextures = process;
    if (ui_.processTexturesCheckBox && ui_.processTexturesCheckBox->isChecked() != processTextures)
        ui_.processTexturesCheckBox->setChecked(processTextures);
}

void AssetInterestPlugin::SetProcessMeshes(bool process)
{
    processMeshes = process;
    if (ui_.processMeshesCheckBox && ui_.processMeshesCheckBox->isChecked() != processMeshes)
        ui_.processMeshesCheckBox->setChecked(processMeshes);
}

void AssetInterestPlugin::SetInspectRemovedEntities(bool inspect)
{
    inspectRemovedEntities = inspect;
    if (ui_.inspectRemovedEntitiesCheckBox && ui_.inspectRemovedEntitiesCheckBox->isChecked() != inspectRemovedEntities)
        ui_.inspectRemovedEntitiesCheckBox->setChecked(inspectRemovedEntities);
}

void AssetInterestPlugin::SetDrawDebug(bool draw)
{
    drawDebug = draw;
    if (ui_.drawDebugCheckBox && ui_.drawDebugCheckBox->isChecked() != drawDebug)
        ui_.drawDebugCheckBox->setChecked(drawDebug);
}

void AssetInterestPlugin::SetInterestRadius(double radius)
{
    interestRadius = radius;

    // We want this function to also update the ui as it may be called by scripts.
    // We also want to avoid infinite loops hence lets make sure of it with this valueChanged disconnect.
    if (ui_.radiusDoubleSpinBox && ui_.radiusDoubleSpinBox->value() != interestRadius)
    {
        disconnect(ui_.radiusDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(SetInterestRadius(double)));
        ui_.radiusDoubleSpinBox->setValue(interestRadius);
        connect(ui_.radiusDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(SetInterestRadius(double)));
    }
}

void AssetInterestPlugin::SetWaitAfterLoad(int intervalMsec)
{
    waitAfterLoad = intervalMsec < 1 ? 1 : intervalMsec;

    // We want this function to also update the ui as it may be called by scripts.
    // We also want to avoid infinite loops hence lets make sure of it with this valueChanged disconnect.
    if (ui_.loadIntervalSpinBox && ui_.loadIntervalSpinBox->value() != waitAfterLoad)
    {
        disconnect(ui_.loadIntervalSpinBox, SIGNAL(valueChanged(int)), this, SLOT(SetWaitAfterLoad(int)));
        ui_.loadIntervalSpinBox->setValue(waitAfterLoad);
        connect(ui_.loadIntervalSpinBox, SIGNAL(valueChanged(int)), this, SLOT(SetWaitAfterLoad(int)));
    }
}

void AssetInterestPlugin::LoadEverythingBack()
{
    if (!Connected())
        return;

    // Get needed data
    Entity *cameraEnt = MainCamera();
    if (!cameraEnt || !cameraEnt->ParentScene())
        return;
    EC_Camera *camera = dynamic_cast<EC_Camera*>(cameraEnt->GetComponent(EC_Camera::TypeNameStatic()).get());
    EC_Placeable *placeable = dynamic_cast<EC_Placeable*>(cameraEnt->GetComponent(EC_Placeable::TypeNameStatic()).get());
    if (!camera || !placeable)
        return;
    float3 cameraPos = placeable->WorldPosition();

    QSet<QString> refsToLoad;
    
    // Iterate scene
    Scene *scene = cameraEnt->ParentScene();
    Scene::EntityMap::const_iterator iter = scene->begin();
    Scene::EntityMap::const_iterator end = scene->end();
    while (iter != end)
    {
        Entity *ent = iter->second.get();
        ++iter;
        if (!ent)
            continue;

        // We assume there is a single placeable and mesh for now
        EC_Placeable *entP = dynamic_cast<EC_Placeable*>(ent->GetComponent(EC_Placeable::TypeNameStatic()).get());
        EC_Mesh *entM = dynamic_cast<EC_Mesh*>(ent->GetComponent(EC_Mesh::TypeNameStatic()).get());
        if (!entP || !entM)
            continue;

        // Materials and textures
        AssetReferenceList mats = entM->getmeshMaterial();
        for (int iMat=0; iMat<mats.Size(); iMat++)
        {
            QString ref = mats[iMat].ref;
            if (!ShouldProcess(ref))
                continue;

            OgreMaterialAsset *matAsset = dynamic_cast<OgreMaterialAsset*>(GetFramework()->Asset()->GetAsset(ref).get());
            if (matAsset && !matAsset->IsLoaded() && !refsToLoad.contains(ref))
                refsToLoad << ref;
        }

        // Meshes
        QString ref = entM->getmeshRef().ref;
        if (ShouldProcess(ref))
        {
            OgreMeshAsset *meshAsset = dynamic_cast<OgreMeshAsset*>(GetFramework()->Asset()->GetAsset(ref).get());
            if (meshAsset && !meshAsset->IsLoaded() && !refsToLoad.contains(ref))
                refsToLoad << ref;
        }
    }

    foreach(QString ref, refsToLoad)
        GetFramework()->Asset()->RequestAsset(ref);

    if (widget_)
    {
        ui_.loadMaterial->setText("none");
        ui_.loadMesh->setText("none");
        ui_.unloadMaterial->setText("none");
        ui_.unloadMesh->setText("none");
        ui_.meshManagerStatus->setText("-");
        ui_.textureManagerStatus->setText("-");
    }
}

extern "C"
{
    DLLEXPORT void TundraPluginMain(Framework *fw)
    {
        Framework::SetInstance(fw); // Inside this DLL, remember the pointer to the global framework object.
        IModule *module = new AssetInterestPlugin();
        fw->RegisterModule(module);
    }
}

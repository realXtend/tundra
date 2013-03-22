// For conditions of distribution and use, see copyright notice in LICENSE

#include "Math/MathNamespace.h"
#include "DebugOperatorNew.h"
#include "EC_SlideShow.h"
#include "SceneWidgetComponents.h"

#include "Framework.h"
#include "SceneAPI.h"
#include "SceneInteract.h"
#include "Entity.h"
#include "AttributeMetadata.h"
#include "AssetAPI.h"
#include "IAsset.h"
#include "AssetRefListener.h"
#include "IAssetTransfer.h"
#include "UiAPI.h"
#include "UiMainWindow.h"

#include "EC_WidgetCanvas.h"
#include "EC_Mesh.h"
#include "TextureAsset.h"
#include "OgreMaterialManager.h"
#include "OgreMaterial.h"
#include "OgreTechnique.h"
#include "OgrePass.h"
#include "OgreTextureUnitState.h"
#include "IRenderer.h"
#include "TundraLogicModule.h"

#include <QPainter>
#include <QColor>
#include <QFont>
#include <QUuid>
#include <QStringList>

#include "LoggingFunctions.h"
#include "MemoryLeakCheck.h"

EC_SlideShow::EC_SlideShow(Scene *scene) :
    IComponent(scene),
    slides(this, "Slides", QVariantList()),
    slideChangeInterval(this, "Change Interval", 0),
    currentSlideIndex(this, "Current Slide", 0),
    renderSubmeshIndex(this, "Render Submesh", 0),
    enabled(this, "Enabled", true),
    interactive(this, "Interactive", false),
    illuminating(this, "Illuminating", true),
    isServer_(false),
    currentTextureRef_("")
{
    static AttributeMetadata zeroIndexMetadata;
    static AttributeMetadata slideIndexMetadata;
    static bool metadataInitialized = false;
    if (!metadataInitialized)
    {
        zeroIndexMetadata.minimum = "0";
        zeroIndexMetadata.step = "1";
        slideIndexMetadata.minimum = "-1";
        slideIndexMetadata.step = "1";
        metadataInitialized = true;
    }
    renderSubmeshIndex.SetMetadata(&zeroIndexMetadata);
    slideChangeInterval.SetMetadata(&zeroIndexMetadata);
    currentSlideIndex.SetMetadata(&slideIndexMetadata);

    // Connect signals both for headless and non-headless
    connect(&changeTimer_, SIGNAL(timeout()), SLOT(NextSlide()));

    // Server controls the slide change timer, so detect if we are on one.
    TundraLogic::TundraLogicModule *tundraLogic = framework->GetModule<TundraLogic::TundraLogicModule>();
    if (tundraLogic)
        isServer_ = tundraLogic->IsServer();
    else
        LogError("EC_SlideShow: Could not detect if in server on client. Slide change timer will not work. (TundraLogicModule not found!)");

    // Don't do anything beyond if rendering is not enabled
    if (!ViewEnabled() || GetFramework()->IsHeadless())
        return;

    // Connect signals for non headless
    connect(this, SIGNAL(ParentEntitySet()), SLOT(PrepareComponent()), Qt::UniqueConnection);
    
    // Prepare scene interactions
    SceneInteract *sceneInteract = GetFramework()->GetModule<SceneInteract>();
    if (sceneInteract)
        connect(sceneInteract, SIGNAL(EntityClicked(Entity*, Qt::MouseButton, RaycastResult*)), SLOT(EntityClicked(Entity*, Qt::MouseButton, RaycastResult*)));
    
    // Handle window resizing to update the rendering (otherwise will get black screen on manually blitted textures)
    if (GetFramework()->Ui()->MainWindow())
    {
        resizeRenderTimer_.setSingleShot(true);
        connect(&resizeRenderTimer_, SIGNAL(timeout()), SLOT(ResizeTimeout()), Qt::UniqueConnection);
        connect(GetFramework()->Ui()->MainWindow(), SIGNAL(WindowResizeEvent(int,int)), SLOT(WindowResized()), Qt::UniqueConnection);
    }

    // Monitor unloaded assets from AssetAPI
    connect(framework->Asset(), SIGNAL(AssetAboutToBeRemoved(AssetPtr)), SLOT(AssetRemoved(AssetPtr)));
}

EC_SlideShow::~EC_SlideShow()
{
}

void EC_SlideShow::ShowSlide(int index)
{
    if (!IsPrepared())
        return;
    if (!getenabled())
        return;

    QVariantList slideRefs = getslides();
    if (slideRefs.empty())
        return;
    if (index < 0)
    {
        QTimer::singleShot(10, this, SLOT(GoToEnd()));
        return;
    }
    if (index >= slideRefs.length())
    {
        QTimer::singleShot(10, this, SLOT(GoToStart()));
        return;
    }

    // Get scene canvas
    EC_WidgetCanvas *canvas = GetSceneCanvasComponent();
    if (!canvas)
        return;
    canvas->SetSubmesh(getrenderSubmeshIndex());

    SceneWidgetComponents *sceneComponentsPlugin = GetFramework()->GetModule<SceneWidgetComponents>();
    
    // Don't do anything if the ref is not a proper texture, people can put anything into 'slides' list.
    // Lets still do some log warnings so users know to move to the next slide.
    QString slideRef = slideRefs.at(index).toString().trimmed();
    if (slideRef.isEmpty())
    {
        if (sceneComponentsPlugin)
            canvas->Update(sceneComponentsPlugin->DrawMessageTexture(QString("No texture set for slide %1").arg(index), false));
        return;
    }

    // Loop and find out asset ref, we cant do by index if there are 
    // empty refs in the 'slides' or non Texture type refs.
    foreach(AssetRefListener *listener, assetListeners_)
    {
        if (!listener)
            return;

        // Find the correct texture for this slide
        int textureIndex = listener->property("slideIndex").toInt();
        if (textureIndex != index)
            continue;
        QString textureRef = listener->property("textureRef").toString();
        bool tranferFailed = listener->property("transferFailed").toBool();
        bool isEmpty = listener->property("isEmpty").toBool();
        bool isTexture = listener->property("isTexture").toBool();
        bool isRequested = listener->property("isRequested").toBool();

        // Empty ref
        if (isEmpty || textureRef.trimmed().isEmpty())
        {
            if (sceneComponentsPlugin)
                canvas->Update(sceneComponentsPlugin->DrawMessageTexture(QString("No texture set for slide %1").arg(index), false));
            return;
        }
        // Non texture ref
        if (!isTexture)
        {
            if (sceneComponentsPlugin)
                canvas->Update(sceneComponentsPlugin->DrawMessageTexture(QString("Slide %1 reference is not a texture").arg(index), true));
            return;
        }
        // Not requested yet
        if (!isRequested)
        {
            listener->setProperty("isRequested", true);
            listener->setProperty("transferFailed", false);
            listener->HandleAssetRefChange(framework->Asset(), textureRef, "Texture");
            return;
        }
        // Asset is null, but transfer has not failed. Wait.
        if (!listener->Asset().get() && !tranferFailed)
        {
            if (sceneComponentsPlugin)
                canvas->Update(sceneComponentsPlugin->DrawMessageTexture(QString("Downloading texture for slide %1...").arg(index), false));
            return;
        }
        // Asset is null and transfer had failed. Draw informative texture to target.
        if (!listener->Asset().get() && tranferFailed)
        {
            if (sceneComponentsPlugin)
                canvas->Update(sceneComponentsPlugin->DrawMessageTexture(QString("Texture download failed for slide %1").arg(index), true));
            return;
        }
        // The above ones should always cover this situation.
        if (!listener->Asset().get())
            return;
        if (!listener->Asset()->IsLoaded())
        {
            if (sceneComponentsPlugin)
                canvas->Update(sceneComponentsPlugin->DrawMessageTexture(QString("Loading texture for slide %1...").arg(index), false));
            return;
        }

        // This should never get here having a listener with non 'Texture' type assets, but double check.
        TextureAsset *textureAsset = dynamic_cast<TextureAsset*>(listener->Asset().get());
        if (!textureAsset)
        {
            // If the asset has been loaded with != "Texture" type but is infact texture when the ref
            // is looked up something has loaded this with improper type before it was set to this SlideShow.
            // We can easily fix this situation by forgetting the non "Texture" AssetPtr and fetching it again.
            // In fact we know the type is "Texture" if we got this far in the check chain!
            if (framework->Asset()->GetResourceTypeFromAssetRef(textureRef) == "Texture")
            {
                framework->Asset()->ForgetAsset(textureRef, false);
                listener->setProperty("isRequested", true);
                listener->setProperty("transferFailed", false);
                listener->HandleAssetRefChange(framework->Asset(), textureRef, "Texture");
            }
            else
                LogWarning("EC_SlideShow: My slide assets list seems to have non 'Texture' type asset reference! " + listener->Asset()->Name());
            return;
        }
        
        const QString applyingTextureRef = listener->Asset()->Name();
        const QString ogreTextureName = textureAsset->ogreAssetName;
        Ogre::TextureUnitState *textureUnit = GetRenderTextureUnit();
        if (textureUnit)
        {
            textureUnit->setTextureName(ogreTextureName.toStdString());

            // Unload previously set texture from asset system and Ogre to save memory.
            // We don't remove the disk source so its fast to load back.
            if (currentTextureRef_ != applyingTextureRef && !currentTextureRef_.isEmpty())
            {
                framework->Asset()->ForgetAsset(currentTextureRef_, false);

                // If this ref is still handled by a listener, update the state
                foreach(AssetRefListener *iterListener, assetListeners_)
                {
                    if (iterListener && iterListener->Asset().get() && iterListener->Asset()->Name() == currentTextureRef_)
                    {
                        iterListener->setProperty("isRequested", false);
                        iterListener->setProperty("transferFailed", false);
                    }
                }
            }
            currentTextureRef_ = applyingTextureRef;
        }
        break;
    }
}

void EC_SlideShow::NextSlide()
{
    // Nothing to do if we have 0 or 1 slides.
    QVariantList slideRefs = getslides();
    if (slideRefs.length() <= 1)
        return;
    // If we are on last slide, ShowSlide() will take care of reseting to 0 index.
    setcurrentSlideIndex(getcurrentSlideIndex() + 1);
}

void EC_SlideShow::PreviousSlide()
{
    // Nothing to do if we have 0 or 1 slides.
    QVariantList slideRefs = getslides();
    if (slideRefs.length() <= 1)
        return;
    // If we are on first slide, ShowSlide() will take care of reseting to len-1 index.
    setcurrentSlideIndex(getcurrentSlideIndex() - 1);
}

void EC_SlideShow::GoToStart()
{
    setcurrentSlideIndex(0);
}

void EC_SlideShow::GoToEnd()
{
    setcurrentSlideIndex(getslides().length() - 1);
}

QMenu *EC_SlideShow::GetContextMenu()
{
    QMenu *actionMenu = new QMenu(0);
    actionMenu->setAttribute(Qt::WA_DeleteOnClose, true);
    actionMenu->addAction(QIcon("./data/ui/images/browser/forward.png"), "Next Slide", this, SLOT(NextSlide()));
    actionMenu->addAction(QIcon("./data/ui/images/browser/back.png"), "Previous Slide", this, SLOT(PreviousSlide()));
    
    QString currentRef = getslides().at(getcurrentSlideIndex()).toString();
    if (!currentRef.isEmpty())
    {
        actionMenu->addSeparator();
        actionMenu->addAction("Current: [" + QString::number(getcurrentSlideIndex()) + "] " + currentRef.split("/").last());
    }
    
    return actionMenu;
}

void EC_SlideShow::WindowResized()
{
#if defined(DIRECTX_ENABLED) && defined(WIN32)
    // Rendering goes black on the texture when 
    // windows is resized only on directx
    if (!resizeRenderTimer_.isActive())
        resizeRenderTimer_.start(500);
#endif
}

void EC_SlideShow::ResizeTimeout()
{
    ShowSlide(getcurrentSlideIndex());
}

void EC_SlideShow::PrepareComponent()
{
    if (framework->IsHeadless())
        return;
    if (IsPrepared())
        return;

    // Get parent and connect to the component removed signal.
    Entity *parent = ParentEntity();
    if (parent)
    {
        connect(parent, SIGNAL(ComponentAdded(IComponent*, AttributeChange::Type)), SLOT(ComponentAdded(IComponent*, AttributeChange::Type)), Qt::UniqueConnection);
        connect(parent, SIGNAL(ComponentRemoved(IComponent*, AttributeChange::Type)), SLOT(ComponentRemoved(IComponent*, AttributeChange::Type)), Qt::UniqueConnection);
    }
    else
    {
        LogError("EC_SlideShow: Could not get parent entity pointer!");
        return;
    }

    // Get EC_Mesh component
    EC_Mesh *mesh = GetMeshComponent();
    if (!mesh)
    {
        // Wait for EC_Mesh to be added with ComponentAdded().
        return;
    }
    else
    {
        // Inspect if this mesh is ready for rendering. EC_Mesh being present != being loaded into Ogre and ready for rendering.
        if (!mesh->GetEntity())
        {
            connect(mesh, SIGNAL(MeshChanged()), SLOT(TargetMeshReady()), Qt::UniqueConnection);
            return;
        }
        else
            connect(mesh, SIGNAL(MaterialChanged(uint, const QString&)), SLOT(TargetMeshMaterialChanged(uint, const QString&)), Qt::UniqueConnection);
    }

    if (sceneCanvasName_.isEmpty())
        sceneCanvasName_ = "SlideShowCanvas-" + QUuid::createUuid().toString().replace("{", "").replace("}", "");

    // Get or create local EC_WidgetCanvas component
    shared_ptr<EC_WidgetCanvas> sceneCanvas = parent->GetOrCreateComponent<EC_WidgetCanvas>(sceneCanvasName_, AttributeChange::LocalOnly, false);
    if (!sceneCanvas)
    {
        LogError("EC_SlideShow: Could not get or create EC_WidgetCanvas component!");
        return;
    }
    sceneCanvas->SetTemporary(true);
    sceneCanvas->SetSelfIllumination(getilluminating());

    // Set submesh to EC_WidgetCanvas if different from current
    if (!sceneCanvas->GetSubMeshes().contains(getrenderSubmeshIndex()))
        sceneCanvas->SetSubmesh(getrenderSubmeshIndex());

    // We are now prepared, check enabled state and restore possible materials now.
    // If enabled try to show current slide.
    if (!getenabled())
        sceneCanvas->RestoreOriginalMeshMaterials();
    else
        ShowSlide(getcurrentSlideIndex());
}

Ogre::TextureUnitState *EC_SlideShow::GetRenderTextureUnit()
{
    EC_WidgetCanvas *canvas = GetSceneCanvasComponent();
    if (!canvas)
        return 0;

    const QString ogreMaterialName = canvas->GetMaterialName();
    Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().getByName(ogreMaterialName.toStdString());
    if (!material.isNull())
    {
        if (!material->getTechnique(0) || !material->getTechnique(0)->getPass(0) ||
            !material->getTechnique(0)->getPass(0)->getTextureUnitState(0))
        {
            LogError("EC_SlideShow: Seems that EC_WidgetCanvas has failed to create our material tech, pass or texture unit!");
            return 0;
        }

        return material->getTechnique(0)->getPass(0)->getTextureUnitState(0);
    }
    return 0;
}

void EC_SlideShow::TextureLoaded(AssetPtr asset)
{
    if (!IsPrepared())
        return;

    int currentSlideIndex = getcurrentSlideIndex();
    QString loadedRef = asset->Name();

    foreach(AssetRefListener *listener, assetListeners_)
    {
        if (!listener)
            continue;

        QString textureRef = listener->property("textureRef").toString();
        int slideIndex = listener->property("slideIndex").toInt();
        if (textureRef == loadedRef && slideIndex == currentSlideIndex)
        {
            // If loaded texture is the current slide index, update the rendering
            ShowSlide(currentSlideIndex);
            break;
        }
    }
}

void EC_SlideShow::TextureLoadFailed(IAssetTransfer *transfer, QString reason)
{
    int currentSlideIndex = getcurrentSlideIndex();
    QString failedRef = transfer->SourceUrl();
    foreach(AssetRefListener *listener, assetListeners_)
    {
        if (!listener)
            continue;

        QString textureRef = listener->property("textureRef").toString();
        int slideIndex = listener->property("slideIndex").toInt();
        if (textureRef == failedRef)
        {
            listener->setProperty("transferFailed", true);

            // If failed texture is the current index, update the rendering
            if (currentSlideIndex == slideIndex)
                ShowSlide(currentSlideIndex);
        }
    }
}

void EC_SlideShow::AssetRemoved(AssetPtr asset)
{
    QString assetRef = asset->Name();
    
    // We are only interested in texture typed assets
    if (framework->Asset()->GetResourceTypeFromAssetRef(assetRef) != "Texture")
        return;
    
    foreach(AssetRefListener *listener, assetListeners_)
    {
        if (!listener)
            continue;

        QString textureRef = listener->property("textureRef").toString();
        if (textureRef == assetRef)
        {
            listener->setProperty("isRequested", false);
            listener->setProperty("transferFailed", false);
        }
    }
}

bool EC_SlideShow::IsPrepared()
{
    if (framework->IsHeadless())
        return false;
    if (!ParentEntity())
        return false;
    EC_Mesh *mesh = GetMeshComponent();
    if (!mesh)
        return false;
    else if (!mesh->GetEntity())
        return false;
    EC_WidgetCanvas *canvas  = GetSceneCanvasComponent();
    if (!canvas)
        return false;
    return true;
}

void EC_SlideShow::TargetMeshReady()
{
    if (!IsPrepared())
        PrepareComponent();
}

void EC_SlideShow::TargetMeshMaterialChanged(uint index, const QString &material)
{
    if (!IsPrepared())
        return;
    if (!getenabled())
        return;

    if (index == (uint)getrenderSubmeshIndex())
    {
        EC_WidgetCanvas *sceneCanvas = GetSceneCanvasComponent();
        if (sceneCanvas)
        {
            if (material != sceneCanvas->GetMaterialName())
            {
                // This will make 3DCanvas to update its internals, which means
                // our material is re-applied to the submesh.
                sceneCanvas->SetSubmesh(getrenderSubmeshIndex());
            }
        }
    }
}

void EC_SlideShow::ResetSubmeshIndex()
{
    setrenderSubmeshIndex(0);
}

void EC_SlideShow::ComponentAdded(IComponent *component, AttributeChange::Type change)
{
    if (component->TypeName() == EC_Mesh::TypeNameStatic())
    {
        if (!IsPrepared())
            PrepareComponent();
    }
}

void EC_SlideShow::ComponentRemoved(IComponent *component, AttributeChange::Type change)
{
    // If this component is being removed we need to reset to the target meshes original materials and remove the comp.
    if (component == this)
    {
        // Reset EC_WidgetCanvas
        EC_WidgetCanvas *canvasSource = GetSceneCanvasComponent();
        if (canvasSource)
        {
            canvasSource->RestoreOriginalMeshMaterials();
            canvasSource->SetWidget(0);
        }

        // Clean up our EC_WidgetCanvas component from the entity
        if (ParentEntity() && !sceneCanvasName_.isEmpty())
            ParentEntity()->RemoveComponent(EC_WidgetCanvas::TypeNameStatic(), sceneCanvasName_, AttributeChange::LocalOnly);
    }
}

void EC_SlideShow::AttributesChanged()
{
    if (slideChangeInterval.ValueChanged())
    {
        if (isServer_)
        {
            int timerSec = getslideChangeInterval();
            if (timerSec <= 0)
                changeTimer_.stop();
            else
                changeTimer_.start(timerSec * 1000);
        }
    }
    if (framework->IsHeadless())
        return;

    // Can handle before we are prepared
    if (slides.ValueChanged())
    {
        // Don't request textures on the server or headless
        if (isServer_ || framework->IsHeadless())
            return;

        foreach(AssetRefListener *listener, assetListeners_)
            SAFE_DELETE(listener);
        assetListeners_.clear();
        QVariantList newSlides = getslides();

        for (int i=0; i<newSlides.size(); i++)
        {
            QString slideRef = newSlides.at(i).toString().trimmed();

            // New listener
            AssetRefListener *listener = new AssetRefListener();

            // Set dynamic properties
            listener->setProperty("slideIndex", i);
            listener->setProperty("transferFailed", false);
            listener->setProperty("textureRef", slideRef);
            listener->setProperty("isRequested", false);
            listener->setProperty("isEmpty", false);
            listener->setProperty("isTexture", true);
            assetListeners_.append(listener);

            // Stop here if empty ref or not a texture
            if (slideRef.isEmpty())
            {
                listener->setProperty("isEmpty", true);
                continue;
            }
            if (framework->Asset()->GetResourceTypeFromAssetRef(slideRef) != "Texture")
            {
                listener->setProperty("isTexture", false);
                continue;
            }

            // Connect signals
            connect(listener, SIGNAL(Loaded(AssetPtr)), SLOT(TextureLoaded(AssetPtr)));
            connect(listener, SIGNAL(TransferFailed(IAssetTransfer*, QString)), SLOT(TextureLoadFailed(IAssetTransfer*, QString)));

            // Request the asset only for the current index
            if (i == getcurrentSlideIndex())
            {
                listener->setProperty("isRequested", true);
                listener->HandleAssetRefChange(framework->Asset(), slideRef, "Texture");
            }
        }
    }
        
    // Cant handle yet, not prepared
    if (!IsPrepared())
        return;

    if (currentSlideIndex.ValueChanged())
    {
        if (getenabled())
            ShowSlide(getcurrentSlideIndex());
    }
    if (renderSubmeshIndex.ValueChanged())
    {
        EC_Mesh *mesh = GetMeshComponent();
        // Validate submesh index from EC_Mesh
        uint submeshIndex = (uint)getrenderSubmeshIndex();
        if (submeshIndex >= mesh->GetNumSubMeshes())
        {
            /// \note ResetSubmeshIndex() is called with a small delay here, or the ec editor UI wont react to it. Resetting the index back to 0 will call Render() again.
            LogWarning("Render submesh index " + QString::number(submeshIndex) + " is illegal, restoring default value.");
            QTimer::singleShot(10, this, SLOT(ResetSubmeshIndex()));
            return;
        }

        EC_WidgetCanvas *sceneCanvas = GetSceneCanvasComponent();
        // Set submesh to EC_WidgetCanvas if different from current
        if (!sceneCanvas->GetSubMeshes().contains(submeshIndex))
            sceneCanvas->SetSubmesh(submeshIndex);
    }
    if (enabled.ValueChanged())
    {
        EC_WidgetCanvas *sceneCanvas = GetSceneCanvasComponent();
        if (!getenabled())
            sceneCanvas->RestoreOriginalMeshMaterials();
        else
        {
            sceneCanvas->SetSubmesh(getrenderSubmeshIndex());
            ShowSlide(getcurrentSlideIndex());
        }
    }
    if (illuminating.ValueChanged())
    {
        EC_WidgetCanvas *canvas = GetSceneCanvasComponent();
        if (canvas)
            canvas->SetSelfIllumination(getilluminating());
    }
}

EC_Mesh *EC_SlideShow::GetMeshComponent()
{
    if (!ParentEntity())
        return 0;
    EC_Mesh *mesh = ParentEntity()->GetComponent<EC_Mesh>().get();
    return mesh;
}

EC_WidgetCanvas *EC_SlideShow::GetSceneCanvasComponent()
{
    if (!ParentEntity())
        return 0;
    if (sceneCanvasName_.isEmpty())
        return 0;
    IComponent *comp = ParentEntity()->GetComponent(EC_WidgetCanvas::TypeNameStatic(), sceneCanvasName_).get();
    EC_WidgetCanvas *sceneCanvas = dynamic_cast<EC_WidgetCanvas*>(comp);
    return sceneCanvas;
}

void EC_SlideShow::EntityClicked(Entity *entity, Qt::MouseButton button, RaycastResult *raycastResult)
{
    if (!getinteractive())
        return;
    if (!IsPrepared())
        return;

    // We are only interested in left clicks on our entity.
    if (!raycastResult)
        return;
    if (button != Qt::LeftButton)
        return;

    if (entity == ParentEntity())
    {
        // We are only interested in clicks to our target submesh index.
        if (raycastResult->submesh != (unsigned)getrenderSubmeshIndex())
            return;

        // Entities have EC_Highlight if it is being manipulated.
        // At this situation we don't want to show any ui.
        if (entity->GetComponent("EC_Highlight"))
            return;

        QMenu *contextMenu = GetContextMenu();
        if (!contextMenu->actions().empty())
            contextMenu->popup(QCursor::pos());
    }
}

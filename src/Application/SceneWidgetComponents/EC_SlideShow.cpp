// For conditions of distribution and use, see copyright notice in license.txt

#include "DebugOperatorNew.h"
#include "EC_SlideShow.h"

#include "Framework.h"
#include "SceneAPI.h"
#include "SceneInteract.h"
#include "Entity.h"
#include "AttributeMetadata.h"
#include "AssetAPI.h"
#include "IAsset.h"
#include "AssetRefListener.h"

#include "EC_WidgetCanvas.h"
#include "EC_Mesh.h"
#include "TextureAsset.h"
#include "OgreMaterialManager.h"
#include "OgreMaterial.h"
#include "OgreTechnique.h"
#include "OgrePass.h"
#include "OgreTextureUnitState.h"
#include "IRenderer.h"

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
    illuminating(this, "Illuminating", true)
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

    // Don't do anything beyond if rendering is not enabled
    if (!ViewEnabled() || GetFramework()->IsHeadless())
        return;

    // Connect signals from IComponent
    connect(this, SIGNAL(ParentEntitySet()), SLOT(PrepareComponent()), Qt::UniqueConnection);
    connect(this, SIGNAL(AttributeChanged(IAttribute*, AttributeChange::Type)), SLOT(AttributeChanged(IAttribute*, AttributeChange::Type)), Qt::UniqueConnection);

    // Prepare scene interactions
    SceneInteract *sceneInteract = GetFramework()->Scene()->GetSceneInteract();
    if (sceneInteract)
    {
        connect(sceneInteract, SIGNAL(EntityClicked(Entity*, Qt::MouseButton, RaycastResult*)), 
                SLOT(EntityClicked(Entity*, Qt::MouseButton, RaycastResult*)));
    }

    connect(&changeTimer_, SIGNAL(timeout()), SLOT(NextSlide()));
}

EC_SlideShow::~EC_SlideShow()
{
}

void EC_SlideShow::ShowSlide(int index)
{
    if (!IsPrepared())
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

    // Don't do anything if the ref is not a proper texture, people can put anything into 'slides' list.
    // Lets still do some log warnings so users know to move to the next slide.
    QString slideRef = slideRefs.at(index).toString();
    if (slideRef.isEmpty())
        return;
    if (framework->Asset()->GetResourceTypeFromAssetRef(slideRef) != "Texture")
    {
        LogWarning("EC_SlideShow: Index " + QString::number(index) + " slide ref is not a texture.");
        return;
    }

    // Loop and find out asset ref, we cant do by index if there are 
    // empty refs in the 'slides' or non Texture type refs.
    foreach(AssetRefListener *listener, assetListeners_)
    {
        if (!listener)
            return;
        if (!listener->Asset().get())
            return;
        if (!listener->Asset()->IsLoaded())
            return;

        // This should never get here having a listener with non 'Texture' type assets, but double check.
        TextureAsset *textureAsset = dynamic_cast<TextureAsset*>(listener->Asset().get());
        if (!textureAsset)
        {
            LogWarning("EC_SlideShow: My slide assets list seems to have non 'Texture' asset reference!");
            continue;               
        }

        if (textureAsset->Name() == slideRef)
        {
            EC_WidgetCanvas *canvas = GetSceneCanvasComponent();
            if (!canvas)
                return;

            const QString ogreTextureName = textureAsset->ogreAssetName;
            const QString ogreMaterialName = canvas->GetMaterialName();
            Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().getByName(ogreMaterialName.toStdString());
            if (!material.isNull())
            {
                if (!material->getTechnique(0) || !material->getTechnique(0)->getPass(0) ||
                    !material->getTechnique(0)->getPass(0)->getTextureUnitState(0))
                {
                    LogError("EC_SlideShow: Seems that EC_WidgetCanvas has failed to create our material tech, pass or texture unit!");
                    return;
                }

                Ogre::TextureUnitState *textureUnit = material->getTechnique(0)->getPass(0)->getTextureUnitState(0);
                textureUnit->setTextureName(ogreTextureName.toStdString());
            }
            break;
        }
    }

}

void EC_SlideShow::NextSlide()
{
    if (!IsPrepared())
        return;
    // Nothing to do if we have 0 or 1 slides.
    QVariantList slideRefs = getslides();
    if (slideRefs.length() <= 1)
        return;
    // If we are on last slide, ShowSlide() will take care of reseting to 0 index.
    setcurrentSlideIndex(getcurrentSlideIndex() + 1);
}

void EC_SlideShow::PreviousSlide()
{
    if (!IsPrepared())
        return;
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

void EC_SlideShow::PrepareComponent()
{
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
        LogError("EC_MediaPlayer: Could not get parent entity pointer!");
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
    ComponentPtr iComponent = parent->GetOrCreateComponent(EC_WidgetCanvas::TypeNameStatic(), sceneCanvasName_, AttributeChange::LocalOnly, false);
    EC_WidgetCanvas *sceneCanvas = dynamic_cast<EC_WidgetCanvas*>(iComponent.get());
    if (!sceneCanvas)
    {
        LogError("EC_MediaPlayer: Could not get or create EC_WidgetCanvas component!");
        return;
    }
    sceneCanvas->SetTemporary(true);
    sceneCanvas->SetSelfIllumination(getilluminating());

    // Set submesh to EC_WidgetCanvas if different from current
    if (!sceneCanvas->GetSubMeshes().contains(getrenderSubmeshIndex()))
        sceneCanvas->SetSubmesh(getrenderSubmeshIndex());

    // We are now prepared, check enabled state and restore possible materials now
    if (!getenabled())
        sceneCanvas->RestoreOriginalMeshMaterials();
}

void EC_SlideShow::TextureLoaded(AssetPtr asset)
{
    if (!IsPrepared())
        return;

    // Check if the loaded texture is the current index
    QString slideRef = asset->Name();
    QVariantList slideRefs = getslides();
    if (getcurrentSlideIndex() >= slideRefs.length())
        return;
    QVariant currentSlideRef = slideRefs.at(getcurrentSlideIndex());
    if (slideRef == currentSlideRef.toString())
        ShowSlide(getcurrentSlideIndex());
}

bool EC_SlideShow::IsPrepared()
{
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

void EC_SlideShow::AttributeChanged(IAttribute *attribute, AttributeChange::Type changeType)
{
    // Can handle before we are prepared
    if (attribute == &slides)
    {
        foreach(AssetRefListener *listener, assetListeners_)
            SAFE_DELETE(listener);
        assetListeners_.clear();
        QVariantList newSlides = getslides();
        foreach(QVariant slideRefVar, newSlides)
        {
            if (slideRefVar.isNull())
                continue;
            QString slideRef = slideRefVar.toString();
            if (slideRef.isEmpty())
                continue;
            if (framework->Asset()->GetResourceTypeFromAssetRef(slideRef) != "Texture")
            {
                LogWarning("EC_SlideShow: Given slide is not a texture: " + slideRef);
                continue;
            }
            AssetRefListener *listener = new AssetRefListener();
            connect(listener, SIGNAL(Loaded(AssetPtr)), SLOT(TextureLoaded(AssetPtr)));
            listener->HandleAssetRefChange(framework->Asset(), slideRef);
            assetListeners_.append(listener);
        }
    }
    else if (attribute == &slideChangeInterval)
    {
        int timerSec = getslideChangeInterval();
        if (timerSec <= 0)
            changeTimer_.stop();
        else
            changeTimer_.start(timerSec * 1000);
    }

    // Cant handle yet, not prepared
    if (!IsPrepared())
        return;

    if (attribute == &currentSlideIndex)
    {
        if (getenabled())
            ShowSlide(getcurrentSlideIndex());
    }
    else if (attribute == &renderSubmeshIndex)
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
    else if (attribute == &enabled)
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
    else if (attribute == &illuminating)
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

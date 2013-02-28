// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"

#include "UiPlane.h"
#include "OgreMaterialAsset.h"
#include "TextureAsset.h"
#include "RenderWindow.h"

#include "Math/MathFunc.h"
#include "LoggingFunctions.h"

#include <OgreOverlay.h>
#include <OgreOverlayManager.h>
#include <OgrePanelOverlayElement.h>

UiPlane::UiPlane(Framework *framework, RenderWindow *renderWindow)
:overlay(0), overlayElement(0), anchorX(-1), anchorY(-1), fw(framework), rw(renderWindow)
{
    x.value = 0.f;
    y.value = 0.f;
    width.value = 1.f;
    height.value = 1.f;

    connect(renderWindow, SIGNAL(Resized(int, int)), this, SLOT(UpdateOgreOverlayGeometry()));
}

UiPlane::~UiPlane()
{
    if (overlay)
    {
        Ogre::OverlayManager::getSingleton().destroy(overlay);
        overlay = 0;
    }
}

void UiPlane::Show()
{
    assert(overlay);
    overlay->show();
}

void UiPlane::Hide()
{
    assert(overlay);
    overlay->hide();
}

void UiPlane::SetZOrder(int zOrder)
{
    assert(overlay);
    if (zOrder < 0)
    {
        LogWarning("UiPlane::SetZOrder: Must specify a Z order >= 0!");
        zOrder = 0;
    }
    else if (zOrder > 650)
    {
        LogWarning("UiPlane::SetZOrder: Must specify a Z order <= 650!");
        zOrder = 650;
    }
    overlay->setZOrder(zOrder);
}

int UiPlane::ZOrder() const
{
    assert(overlay);
    return overlay->getZOrder();
}

void UiPlane::SetAlpha(float alpha_)
{
    OgreMaterialAsset *matAsset = checked_static_cast<OgreMaterialAsset*>(materialAsset.lock().get());
    if (!matAsset)
    {
        LogWarning("UiPlane::SetAlpha failed! no OgreMaterialAsset set!");
        return;
    }
    
    Ogre::Pass *pass = matAsset->ogreMaterial->getTechnique(0)->getPass(0);
    Ogre::TextureUnitState *tu = pass->getTextureUnitState(0);
    alpha = Clamp(alpha_, 0.0f, 1.0f);
    
    pass->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
    tu->setAlphaOperation(Ogre::LBX_BLEND_MANUAL, Ogre::LBS_TEXTURE, Ogre::LBS_MANUAL, 1.0, 0.0, alpha);
}

float UiPlane::Alpha() const
{
    return alpha;
}

void UiPlane::SetX(float x_, bool absolutePixels, int anchorX_)
{
    x.value = x_;
    x.absolute = absolutePixels;
    anchorX = anchorX_;

    UpdateOgreOverlayGeometry();
}

void UiPlane::SetY(float y_, bool absolutePixels, int anchorY_)
{
    y.value = y_;
    y.absolute = absolutePixels;
    anchorY = anchorY_;

    UpdateOgreOverlayGeometry();
}

void UiPlane::SetWidth(float width_, bool absolutePixels)
{
    width.value = width_;
    width.absolute = absolutePixels;

    UpdateOgreOverlayGeometry();
}

void UiPlane::SetHeight(float height_, bool absolutePixels)
{
    height.value = height_;
    height.absolute = absolutePixels;

    UpdateOgreOverlayGeometry();
}

void UiPlane::SetTexture(TextureAsset *texture)
{
    if (texture)
        textureAsset = dynamic_pointer_cast<TextureAsset>(texture->shared_from_this());
    else
        textureAsset.reset();
    UpdateOgreOverlay();
}

void UiPlane::UpdateOgreOverlay()
{
    // Create an Ogre material for the Overlay, if one didn't exist.
    shared_ptr<OgreMaterialAsset> matAsset = materialAsset.lock();
    if (!matAsset)
    {
        AssetAPI *asset = fw->Asset();
        materialAsset = matAsset = dynamic_pointer_cast<OgreMaterialAsset>(asset->CreateNewAsset("OgreMaterial", asset->GenerateUniqueAssetName("Material", "UiPlane")));
        if (!materialAsset.lock())
        {
            LogError("UiPlane::CreateOverlayMaterial: Could not create overlay material asset!");
            return;
        }

        matAsset->SetLighting(0, 0, false);
        matAsset->SetDepthCheck(0, 0, false);
        matAsset->CreateTextureUnit(0, 0);
        matAsset->SetAttribute("tex_address_mode", "clamp");
        matAsset->ogreMaterial->setFog(true, Ogre::FOG_NONE);
        SetAlpha(1.0f); // Default alpha for new UiPlanes.
    }
    
    // Apply the texture onto the material.
    shared_ptr<TextureAsset> texAsset = dynamic_pointer_cast<TextureAsset>(textureAsset.lock());
    if (texAsset)
        matAsset->SetTexture(0, 0, 0, texAsset->Name());

    // Create an Ogre::PanelOverlayElement if one didn't exist.
    if (!overlayElement)
    {
        overlayElement = checked_static_cast<Ogre::PanelOverlayElement*>(Ogre::OverlayManager::getSingleton().createOverlayElement("Panel", fw->Asset()->GenerateUniqueAssetName("Ogre_PanelOverlayElement", "UiPlane").toStdString()));
        overlayElement->setMaterialName(matAsset->ogreAssetName.toStdString());
        overlayElement->setMetricsMode(Ogre::GMM_RELATIVE);
        UpdateOgreOverlayGeometry();
    }

    // Create an Ogre::Overlay if one didn't exist.
    if (!overlay)
    {
        overlay = Ogre::OverlayManager::getSingleton().create(fw->Asset()->GenerateUniqueAssetName("Ogre_PanelOverlayElement", "UiPlane Overlay").toStdString());
        overlay->add2D(static_cast<Ogre::OverlayContainer*>(overlayElement));
        SetZOrder(250); // Default z order for new UiPlanes.
    }
}

void UiPlane::UpdateOgreOverlayGeometry()
{
    const int w = rw->Width();
    const int h = rw->Height();
    assert(overlayElement);
    float relW = width.ToRelative(w);
    float relH = height.ToRelative(h);
    float relX = x.ToRelative(w);
    float relY = y.ToRelative(h);
    if (anchorX == 0)
        relX += 0.5f - relW * 0.5f;
    else if (anchorX == 1)
        relX = 1.f - relX - relW;
    if (anchorY == 0)
        relY += 0.5f - relH * 0.5f;
    else if (anchorY == 1)
        relY = 1.f - relY - relH;
    overlayElement->setDimensions(relW, relH);
    overlayElement->setPosition(relX, relY);
    overlayElement->setUV(0.0f, 0.0f, 1.0f, 1.0f);
}

/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   SceneInteract.cpp
    @brief  Transforms generic mouse and keyboard input events on scene entities to input-related entity actions and signals. */

#include "StableHeaders.h"
#include "SceneInteract.h"

#include "Framework.h"
#include "FrameAPI.h"
#include "InputAPI.h"
#include "IRenderer.h"
#include "Entity.h"
#include "Profiler.h"
#include "OgreRenderingModule.h"
#include "Renderer.h"
#include "OgreWorld.h"

SceneInteract::SceneInteract() :
    IModule("SceneInteract"),
    lastX(-1),
    lastY(-1),
    itemUnderMouse(false),
    frameRaycasted(false),
    lastRaycast(0)
{
}

void SceneInteract::Initialize()
{
    if (!framework_->IsHeadless())
    {
        input = framework_->Input()->RegisterInputContext("SceneInteract", 100);
        input->SetTakeMouseEventsOverQt(true);
        connect(input.get(), SIGNAL(KeyEventReceived(KeyEvent *)), SLOT(HandleKeyEvent(KeyEvent *)));
        connect(input.get(), SIGNAL(MouseEventReceived(MouseEvent *)), SLOT(HandleMouseEvent(MouseEvent *)));
    }
    framework_->RegisterDynamicObject("sceneinteract", this);
}

void SceneInteract::Update(f64 /*frameTime*/)
{
    if (!framework_->IsHeadless())
    {
        PROFILE(SceneInteract_Update);

        ExecuteRaycast();
        if (lastHitEntity.lock())
            lastHitEntity.lock()->Exec(EntityAction::Local, "MouseHover");
            
        frameRaycasted = false;
    }
}

RaycastResult* SceneInteract::CurrentMouseRaycastResult() const
{
    return lastRaycast;
}

RaycastResult* SceneInteract::ExecuteRaycast()
{
    // Return the cached result if already executed this frame.
    if (frameRaycasted && lastRaycast)
        return lastRaycast;
    frameRaycasted = true;

    OgreWorldPtr world = framework_->GetModule<OgreRenderer::OgreRenderingModule>()->GetRenderer()->GetActiveOgreWorld();
    if (!world)
        return 0;

    lastRaycast = world->Raycast(lastX, lastY);
    if (!lastRaycast)
        return 0;
    if (!lastRaycast->entity || itemUnderMouse)
    {
        if (!lastHitEntity.expired())
            lastHitEntity.lock()->Exec(EntityAction::Local, "MouseHoverOut");
        lastHitEntity.reset();
        return lastRaycast;
    }

    EntityPtr lastEntity = lastHitEntity.lock();
    EntityPtr entity = lastRaycast->entity->shared_from_this();
    if (entity != lastEntity)
    {
        if (lastEntity)
            lastEntity->Exec(EntityAction::Local, "MouseHoverOut");

        if (entity)
            entity->Exec(EntityAction::Local, "MouseHoverIn");

        lastHitEntity = entity;
    }

    return lastRaycast;
}

void SceneInteract::HandleKeyEvent(KeyEvent * /*e*/)
{
    /// @todo Evaluate if this is needed at all.
}

void SceneInteract::HandleMouseEvent(MouseEvent *e)
{
    // Invalidate cached raycast if mouse coordinates have changed
    if (frameRaycasted)
        if (lastX != e->x || lastY != e->y)
            frameRaycasted = false;

    lastX = e->x;
    lastY = e->y;
    itemUnderMouse = (e->ItemUnderMouse() != 0);

    RaycastResult *raycastResult = ExecuteRaycast();

    Entity *hitEntity = lastHitEntity.lock().get();
    if (!hitEntity || !raycastResult)
        return;

    if (lastHitEntity.lock())
    {
        switch(e->eventType)
        {
        case MouseEvent::MouseMove:
            emit EntityMouseMove(hitEntity, (Qt::MouseButton)e->button, raycastResult);
            break;
        case MouseEvent::MouseScroll:
            // Execute local entity action with signature:
            // Action name: "MouseScroll"
            // String parameters: (int)"The difference in the mouse wheel position", (float,float,float)"x,y,z", (int)"submesh index"
            /// @todo Uncomment when the hack execution of "MouseScroll" in EC_InputMapper is removed.
            /*
            hitEntity->Exec(EntityAction::Local, "MouseScroll",
                QString::number(e->relativeZ),
                QString("%1,%2,%3").arg(raycastResult->pos.x).arg(raycastResult->pos.y).arg(raycastResult->pos.z),
                QString::number((int)raycastResult->submesh));
            */
            emit EntityMouseScroll(hitEntity, e->relativeZ, raycastResult);
            break;
        case MouseEvent::MousePressed:
        case MouseEvent::MouseDoubleClicked: // For now, double-clicks are just treated as normal clicks.
            // Execute local entity action with signature:
            // Action name: "MousePress"
            // String parameters: (int)"Qt::MouseButton", (float,float,float)"x,y,z", (int)"submesh index"
            hitEntity->Exec(EntityAction::Local, "MousePress",
                            QString::number((uint)e->button),
                            QString("%1,%2,%3").arg(raycastResult->pos.x).arg(raycastResult->pos.y).arg(raycastResult->pos.z),
                            QString::number((int)raycastResult->submesh));
            emit EntityClicked(hitEntity, (Qt::MouseButton)e->button, raycastResult);
            break;
        case MouseEvent::MouseReleased:
            // Execute local entity action with signature:
            // Action name: "MouseRelease"
            // String parameters: (int)"Qt::MouseButton", (float,float,float)"x,y,z", (int)"submesh index"
            hitEntity->Exec(EntityAction::Local, "MouseRelease",
                            QString::number((uint)e->button),
                            QString("%1,%2,%3").arg(raycastResult->pos.x).arg(raycastResult->pos.y).arg(raycastResult->pos.z),
                            QString::number((int)raycastResult->submesh));
            emit EntityClickReleased(hitEntity, (Qt::MouseButton)e->button, raycastResult);
            break;
        default:
            break;
        }
    }
}

extern "C"
{
    DLLEXPORT void TundraPluginMain(Framework *fw)
    {
        Framework::SetInstance(fw); // Inside this DLL, remember the pointer to the global framework object.
        fw->RegisterModule(new SceneInteract());
    }
}

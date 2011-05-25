/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   SceneInteract.cpp
 *  @brief  Transforms generic mouse and keyboard input events to 
 *          input-related Entity Action for scene entities.
 */

#include "StableHeaders.h"
#include "SceneInteract.h"

#include "Framework.h"
#include "FrameAPI.h"
#include "InputAPI.h"
#include "IRenderer.h"
#include "Entity.h"

SceneInteract::SceneInteract() :
    framework(0),
    lastX(-1),
    lastY(-1),
    itemUnderMouse(false)
{
}

void SceneInteract::Initialize(Framework *framework_)
{
    framework = framework_;

    input = framework->Input()->RegisterInputContext("SceneInteract", 100);
    if (input)
    {
        input->SetTakeMouseEventsOverQt(true);

        connect(input.get(), SIGNAL(KeyEventReceived(KeyEvent *)), SLOT(HandleKeyEvent(KeyEvent *)));
        connect(input.get(), SIGNAL(MouseEventReceived(MouseEvent *)), SLOT(HandleMouseEvent(MouseEvent *)));
        connect(framework->Frame(), SIGNAL(Updated(float)), SLOT(Update()));
    }
}

void SceneInteract::Update()
{
    Raycast();

    if (lastHitEntity.lock())
        lastHitEntity.lock()->Exec(EntityAction::Local, "MouseHover");
}

RaycastResult* SceneInteract::Raycast()
{
    IRenderer *renderer = framework->GetRenderer();
    if (!renderer)
        return 0;

    RaycastResult *result = renderer->Raycast(lastX, lastY);
    if (!result)
        return 0;
    
    if (!result->entity || itemUnderMouse)
    {
        if (!lastHitEntity.expired())
            lastHitEntity.lock()->Exec(EntityAction::Local, "MouseHoverOut");
        lastHitEntity.reset();
        return result;
    }

    EntityPtr lastEntity = lastHitEntity.lock();
    EntityPtr entity = result->entity->shared_from_this();
    if (entity != lastEntity)
    {
        if (lastEntity)
            lastEntity->Exec(EntityAction::Local, "MouseHoverOut");

        if (entity)
            entity->Exec(EntityAction::Local, "MouseHoverIn");

        lastHitEntity = entity;
    }

    return result;
}

void SceneInteract::HandleKeyEvent(KeyEvent *e)
{
}

void SceneInteract::HandleMouseEvent(MouseEvent *e)
{
    lastX = e->x;
    lastY = e->y;
    itemUnderMouse = (e->ItemUnderMouse() != 0);

    RaycastResult *raycastResult = Raycast();

    Entity *hitEntity = lastHitEntity.lock().get();
    if (!hitEntity || !raycastResult)
        return;

    if (lastHitEntity.lock())
    {
        /// @todo handle all mouse events properly
        switch(e->eventType)
        {
        case MouseEvent::MouseMove:
            emit EntityMouseMove(hitEntity, (Qt::MouseButton)e->button, raycastResult);
            break;
        case MouseEvent::MouseScroll:
            break;
        case MouseEvent::MousePressed:
        case MouseEvent::MouseDoubleClicked: // For now, double-clicks are just treated as normal clicks.
            // Execute local "MousePress" entity action with signature:
            // Action name: "MousePress"  
            // String parameters: (int)"Qt::MouseButton", (float,float,float)"x,y,z", (int)"submesh index"
            hitEntity->Exec(EntityAction::Local, "MousePress", 
                            QString::number((uint)e->button),
                            QString("%1,%2,%3").arg(QString::number(raycastResult->pos.x), QString::number(raycastResult->pos.y), QString::number(raycastResult->pos.z)),
                            QString::number((int)raycastResult->submesh));

            // Signal signature: EntityClicked(Entity*, Qt::MouseButton, RaycastResult*)
            emit EntityClicked(hitEntity, (Qt::MouseButton)e->button, raycastResult);
            break;
        case MouseEvent::MouseReleased:
            // Execute local "MouseRelease" entity action with signature:
            // Action name: "MouseRelease"  
            // String parameters: (int)"Qt::MouseButton", (float,float,float)"x,y,z", (int)"submesh index"
            hitEntity->Exec(EntityAction::Local, "MouseRelease", 
                            QString::number((uint)e->button),
                            QString("%1,%2,%3").arg(QString::number(raycastResult->pos.x), QString::number(raycastResult->pos.y), QString::number(raycastResult->pos.z)),
                            QString::number((int)raycastResult->submesh));

            emit EntityClickReleased(hitEntity, (Qt::MouseButton)e->button, raycastResult);
            break;
        default:
            break;
        }
    }
}


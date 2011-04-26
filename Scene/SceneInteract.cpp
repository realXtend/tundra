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
#include "RenderServiceInterface.h"
#include "Entity.h"

SceneInteract::SceneInteract() :
    framework_(0),
    lastX_(-1),
    lastY_(-1),
    itemUnderMouse_(false)
{
}

void SceneInteract::Initialize(Framework *framework)
{
    framework_ = framework;

    input_ = framework_->Input()->RegisterInputContext("SceneInteract", 100);
    if (input_)
    {
        input_->SetTakeMouseEventsOverQt(true);

        connect(input_.get(), SIGNAL(KeyEventReceived(KeyEvent *)), SLOT(HandleKeyEvent(KeyEvent *)));
        connect(input_.get(), SIGNAL(MouseEventReceived(MouseEvent *)), SLOT(HandleMouseEvent(MouseEvent *)));
        connect(framework_->Frame(), SIGNAL(Updated(float)), SLOT(Update()));
    }
}

void SceneInteract::PostInitialize()
{
    renderer_ = framework_->GetServiceManager()->GetService<RenderServiceInterface>(Service::ST_Renderer);
}

void SceneInteract::Update()
{
    Raycast();

    if (lastHitEntity_.lock())
        lastHitEntity_.lock()->Exec(EntityAction::Local, "MouseHover");
}

RaycastResult* SceneInteract::Raycast()
{
    if (renderer_.expired())
        return 0;

    RaycastResult* result = renderer_.lock()->Raycast(lastX_, lastY_);
    if (!result->entity_ || itemUnderMouse_)
    {
        if (!lastHitEntity_.expired())
            lastHitEntity_.lock()->Exec(EntityAction::Local, "MouseHoverOut");
        lastHitEntity_.reset();
        return result;
    }

    EntityPtr lastEntity = lastHitEntity_.lock();
    EntityPtr entity = result->entity_->shared_from_this();
    if (entity != lastEntity)
    {
        if (lastEntity)
            lastEntity->Exec(EntityAction::Local, "MouseHoverOut");

        if (entity)
            entity->Exec(EntityAction::Local, "MouseHoverIn");

        lastHitEntity_ = entity;
    }

    return result;
}

void SceneInteract::HandleKeyEvent(KeyEvent *e)
{
}

void SceneInteract::HandleMouseEvent(MouseEvent *e)
{
    lastX_ = e->x;
    lastY_ = e->y;
    itemUnderMouse_ = (e->ItemUnderMouse() != 0);

    RaycastResult *raycastResult = Raycast();

    Entity *hitEntity = lastHitEntity_.lock().get();
    if (!hitEntity || !raycastResult)
        return;

    if (lastHitEntity_.lock())
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
                            QString::number(static_cast<uint>(e->button)),
                            QString("%1,%2,%3").arg(QString::number(raycastResult->pos_.x), QString::number(raycastResult->pos_.y), QString::number(raycastResult->pos_.z)),
                            QString::number(static_cast<int>(raycastResult->submesh_)));

            // Signal signature: EntityClicked(Entity*, Qt::MouseButton, RaycastResult*)
            emit EntityClicked(hitEntity, (Qt::MouseButton)e->button, raycastResult);
            break;
        case MouseEvent::MouseReleased:
            // Execute local "MouseRelease" entity action with signature:
            // Action name: "MouseRelease"  
            // String parameters: (int)"Qt::MouseButton", (float,float,float)"x,y,z", (int)"submesh index"
            hitEntity->Exec(EntityAction::Local, "MouseRelease", 
                            QString::number(static_cast<uint>(e->button)),
                            QString("%1,%2,%3").arg(QString::number(raycastResult->pos_.x), QString::number(raycastResult->pos_.y), QString::number(raycastResult->pos_.z)),
                            QString::number(static_cast<int>(raycastResult->submesh_)));

            emit EntityClickReleased(hitEntity, (Qt::MouseButton)e->button, raycastResult);
            break;
        default:
            break;
        }
    }
}


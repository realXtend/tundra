/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   SceneInteract.cpp
 *  @brief  
 */

#include "StableHeaders.h"
#include "SceneInteract.h"

#include "Framework.h"
#include "Frame.h"
#include "InputServiceInterface.h"
#include "RenderServiceInterface.h"
#include "Entity.h"

SceneInteract::SceneInteract(Foundation::Framework *fw) :
    QObject(fw),
    framework_(fw),
    lastX_(-1),
    lastY_(-1)
{
    renderer_ = framework_->GetServiceManager()->GetService<Foundation::RenderServiceInterface>(Foundation::Service::ST_Renderer);

    input_ = framework_->Input().RegisterInputContext("SceneInterract", 100);
    connect(input_.get(), SIGNAL(OnKeyEvent(KeyEvent *)), SLOT(HandleKeyEvent(KeyEvent *)));
    connect(input_.get(), SIGNAL(OnMouseEvent(MouseEvent *)), SLOT(HandleMouseEvent(MouseEvent *)));

    connect(framework_->GetFrame(), SIGNAL(Updated(float)), SLOT(Update()));
}

void SceneInteract::Update()
{
    Raycast();

    if (lastHitEntity_.lock())
        lastHitEntity_.lock()->Exec("MouseHover");
}

void SceneInteract::Raycast()
{
    if (renderer_.expired())
        return;

    Foundation::RaycastResult result = renderer_.lock()->Raycast(lastX_, lastY_);
    if (!result.entity_)
    {
        if (!lastHitEntity_.expired())
            lastHitEntity_.lock()->Exec("MouseHoverOut");
        lastHitEntity_.reset();
        return;
    }

    Scene::EntityPtr lastEntity = lastHitEntity_.lock();
    Scene::EntityPtr entity = result.entity_->GetSharedPtr();
    if (entity != lastEntity)
    {
        if (lastEntity)
            lastEntity->Exec("MouseHoverOut");

        if (entity)
            entity->Exec("MouseHoverIn");

        lastHitEntity_ = entity;
    }
}

void SceneInteract::HandleKeyEvent(KeyEvent *e)
{
}

void SceneInteract::HandleMouseEvent(MouseEvent *e)
{
    lastX_ = e->x;
    lastY_ = e->y;

    Raycast();

    if (lastHitEntity_.lock())
    {
        if (e->eventType == MouseEvent::MousePressed)
            lastHitEntity_.lock()->Exec("MousePress");

        // handle all mouse events
        //...
    }
}


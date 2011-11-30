// For conditions of distribution and use, see copyright notice in license.txt

#include "SceneWidgetComponents.h"

#include "Framework.h"
#include "Math/float2.h"

#include "InputAPI.h"
#include "MouseEvent.h"
#include "InputContext.h"

#include "SceneAPI.h"
#include "Scene.h"
#include "Entity.h"

#include "UiAPI.h"
#include "UiGraphicsView.h"

#include "EC_WidgetBillboard.h"

SceneWidgetComponents::SceneWidgetComponents() :
    IModule("SceneWidgetComponents")
{
}

SceneWidgetComponents::~SceneWidgetComponents()
{
}

void SceneWidgetComponents::Initialize()
{
    if (!framework_->IsHeadless())
        connect(framework_->Input()->TopLevelInputContext(), SIGNAL(MouseEventReceived(MouseEvent*)), SLOT(OnMouseEvent(MouseEvent*)));
}

void SceneWidgetComponents::OnMouseEvent(MouseEvent *mEvent)
{
    MouseEvent::EventType et = mEvent->eventType;

    if (framework_->IsHeadless())
        return;
    if (!framework_->Scene()->MainCameraScene())
        return;
    if (framework_->Ui()->GraphicsView()->GetVisibleItemAtCoords(mEvent->x, mEvent->y) != 0)
        return;

    // Filter out not wanted events here so we don't 
    // do the potentially costly raycast unnecessarily.
    if (mEvent->handled || mEvent->IsRightButtonDown())
        return;
    else if (et == MouseEvent::MouseScroll)
        return;

    float closestDistance = 100000.0;
    EC_WidgetBillboard *closestComponent = 0;

    EntityList ents = framework_->Scene()->MainCameraScene()->GetEntitiesWithComponent(EC_WidgetBillboard::TypeNameStatic());
    EntityList::const_iterator iter = ents.begin();

    // Find the closest hit EC_WidgetBillboard
    while (iter != ents.end())
    {
        EntityPtr ent = *iter;
        ++iter;

        if (!ent.get())
            continue;

        EC_WidgetBillboard *widgetBillboard = dynamic_cast<EC_WidgetBillboard*>(ent->GetComponent(EC_WidgetBillboard::TypeNameStatic()).get());
        if (!widgetBillboard)
            continue;

        bool hit = false; float2 uv; float distance;
        widgetBillboard->RaycastBillboard(mEvent->x, mEvent->y, hit, uv, distance);

        if (hit)
        {
            if (distance < closestDistance)
            {
                closestDistance = distance;
                closestComponent = widgetBillboard;
            }
        }
        else
            widgetBillboard->CheckMouseState();
    }

    // Redirect mouse event for the closest EC_WidgetBillboard
    if (closestComponent)
    {
        closestComponent->OnMouseEvent(mEvent);
        mEvent->handled = true;
    }
}

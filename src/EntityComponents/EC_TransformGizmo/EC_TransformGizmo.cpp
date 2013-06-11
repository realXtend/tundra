/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   EC_TransformGizmo.cpp
    @brief  Enables visual manipulators (gizmos) for Transform attributes. */

#define MATH_OGRE_INTEROP

#include "EC_TransformGizmo.h"

#include "Framework.h"
#include "Entity.h"
#include "InputAPI.h"
#include "AssetAPI.h"
#include "EC_Placeable.h"
#include "EC_Mesh.h"
#include "LoggingFunctions.h"
#include "OgreRenderingModule.h"
#include "OgreWorld.h"
#include "Scene/Scene.h"
#include "Renderer.h"
#include "EC_Camera.h"
#include "UiAPI.h"
#include "UiGraphicsView.h"
#include "FrameAPI.h"
#include "Profiler.h"
#include "Geometry/Ray.h"
#include "Geometry/Plane.h"

#include <Ogre.h>

namespace
{

const AssetReference cTranslate("Ogre Media:axis1.mesh");
const AssetReference cRotate("Ogre Media:rotate1.mesh");
const AssetReference cScale("Ogre Media:scale1.mesh");

const AssetReference cAxisRed("Ogre Media:AxisRed.material");
const AssetReference cAxisRedHi("Ogre Media:AxisRedHi.material");
const AssetReference cAxisGreen("Ogre Media:AxisGreen.material");
const AssetReference cAxisGreenHi("Ogre Media:AxisGreenHi.material");
const AssetReference cAxisBlue("Ogre Media:AxisBlue.material");
const AssetReference cAxisBlueHi("Ogre Media:AxisBlueHi.material");
const AssetReference cAxisWhite("Ogre Media:AxisWhite.material");

}

EC_TransformGizmo::EC_TransformGizmo(Scene *scene) :
    IComponent(scene),
    gizmoType(Translate),
    state(Inactive),
    prevPoint(float3::zero),
    curPoint(float3::zero),
    worldTM(float3x4::identity),
    worldPos(float3::zero)
{
    connect(this, SIGNAL(ParentEntitySet()), SLOT(Initialize()));

    QString uniqueName("EC_TransformGizmo_" + framework->Asset()->GenerateUniqueAssetName("",""));
    input = framework->Input()->RegisterInputContext(uniqueName, 100);
    connect(input.get(), SIGNAL(MouseEventReceived(MouseEvent *)), SLOT(HandleMouseEvent(MouseEvent *)));

    ogreWorld = scene->GetWorld<OgreWorld>();

    connect(framework->Frame(), SIGNAL(Updated(float)), SLOT(OnFrameUpdate(float)));
}

EC_TransformGizmo::~EC_TransformGizmo()
{
    input.reset();
    placeable.reset();
    mesh.reset();
}

void EC_TransformGizmo::SetPosition(const float3 &pos)
{
    if (placeable)
        placeable->SetPosition(pos);
}

void EC_TransformGizmo::SetOrientation(const Quat &rot)
{
    if (placeable)
        placeable->SetOrientation(rot);
}

void EC_TransformGizmo::SetCurrentGizmoType(GizmoType type)
{
    if (gizmoType == type)
        return;

    gizmoType = type;
    AssetReference ref;
    if (gizmoType == Translate)
        ref = cTranslate;
    if (gizmoType == Rotate)
        ref = cRotate;
    if (gizmoType == Scale)
        ref = cScale;

    if (mesh)
        mesh->meshRef.Set(ref, AttributeChange::Default);
}

void EC_TransformGizmo::SetVisible(bool visible)
{
    if (placeable)
        placeable->visible.Set(visible, AttributeChange::Default);
}

bool EC_TransformGizmo::IsVisible() const
{
    return placeable ? placeable->visible.Get() : false;
}

void EC_TransformGizmo::Initialize()
{
    assert(ParentEntity());
    if (!ParentEntity())
        return;

    ParentEntity()->SetName("TransformGizmo");

    placeable = dynamic_pointer_cast<EC_Placeable>(ParentEntity()->CreateLocalComponent(
        EC_Placeable::TypeNameStatic(), "TransformGizmoPlaceable"));
    if (!placeable)
    {
        LogError("EC_TransformGizmo::Initialize: Could not create EC_Placeable for EC_TransformGizmo.");
        return;
    }

    placeable->SetTemporary(true);
//    placeable->selectionLayer.Set(0, AttributeChange::Default); // ignore raycast
    placeable->visible.Set(false, AttributeChange::Default);

    mesh = dynamic_pointer_cast<EC_Mesh>(ParentEntity()->CreateLocalComponent(
        EC_Mesh::TypeNameStatic(), "TransformGizmoMesh"));
    if (!mesh)
    {
        LogError("EC_TransformGizmo::Initialize: Could not create EC_Mesh for EC_TransformGizmo.");
        return;
    }

    mesh->SetTemporary(true);

    mesh->meshRef.Set(cTranslate, AttributeChange::Default);
    AssetReferenceList materials;
    materials.Append(cAxisGreen);
    materials.Append(cAxisRed);
    materials.Append(cAxisBlue);
    mesh->meshMaterial.Set(materials, AttributeChange::Default);
}

void EC_TransformGizmo::HandleMouseEvent(MouseEvent *e)
{
//    std::cout << IsVisible() << " " << placeable->WorldPosition() << std::endl;
    using namespace OgreRenderer;
    if (!mesh || !placeable)
        return;
    if (!mesh->HasMesh())
        return;
    if (!IsVisible())
    {
        if (input)
            input->ClearMouseCursorOverride();
        return;
    }
    if (ogreWorld.expired())
        return;
    OgreWorldPtr world = ogreWorld.lock();
    if (!world->Renderer()->MainCamera())
        return;
    EC_Camera *cam = world->Renderer()->MainCamera()->GetComponent<EC_Camera>().get();
    if (!cam)
        return;

    bool mouseOnTop = false;
    bool mouseDown = e->IsLeftButtonDown();
    if (e->eventType == MouseEvent::MouseReleased && e->button == MouseEvent::LeftButton)
        mouseDown = false;
    bool mouseDrag = e->eventType == MouseEvent::MouseMove && mouseDown;

    Ray xRay, yRay, zRay;
    // If moving, ignore gizmo's placeable
/// \todo Ali had to comment out the following lines to make gizmo work. Re-enable this. -jj.
//    if ((e->eventType == MouseEvent::MousePressed && e->button == MouseEvent::LeftButton) ||
//        (e->eventType == MouseEvent::MouseMove && e->otherButtons == 0))

    // To avoid confusion while editing, do not update the axes during a drag, although the visualization is updated
    if (state != Active)
    {
        worldTM = placeable->LocalToWorld();
        worldPos = placeable->WorldPosition();
    }
    
    xRay = Ray(worldPos, worldTM.WorldX().Normalized());
    yRay = Ray(worldPos, worldTM.WorldY().Normalized());
    zRay = Ray(worldPos, worldTM.WorldZ().Normalized());

    float relX = (float)e->x/GetFramework()->Ui()->GraphicsView()->size().width();
    float relY = (float)e->y/GetFramework()->Ui()->GraphicsView()->size().height();
    Ray mouseRay = cam->GetMouseRay(relX, relY);

    QList<GizmoAxis> hits;

    float distanceX = mouseRay.Distance(xRay);
    float distanceY = mouseRay.Distance(yRay);
    float distanceZ = mouseRay.Distance(zRay);

    bool hit = false;
    RaycastResult *result = world->Raycast(e->x, e->y);
    if (result->entity && result->entity->GetComponent<EC_TransformGizmo>().get() == this)
        hit = true;


    float offsetX, offsetY, offsetZ;

    // We always keep the transform gizmo constant-sized in the view.
    float3 gizmoPos = mesh->LocalToWorld().TranslatePart();
    float3 cameraPos = placeable->LocalToWorld().TranslatePart();
    const float gizmoScale = DesiredGizmoScale();
    const float cClickOffsetThreshold = gizmoScale; // Specifies how far along the gizmo X,Y and Z axes we can click.
    const float cClickDistanceThreshold = 0.125f * gizmoScale;

    xRay.ClosestPoint(mouseRay, &offsetX);
    if ((distanceX < cClickDistanceThreshold && offsetX > 0 && offsetX < cClickOffsetThreshold) ||
        (hit && result->submesh == GizmoAxis::X))
    {
        mouseOnTop = true;
        if (state != Active)
        {
            GizmoAxis x;
            x.axis = GizmoAxis::X;
            x.material = mouseDown ? cAxisWhite : cAxisRedHi;
            x.ray = xRay;
            hits.append(x);
        }
    }

    yRay.ClosestPoint(mouseRay, &offsetY);
    if ((distanceY < cClickDistanceThreshold && offsetY > 0 && offsetY < cClickOffsetThreshold) ||
        (hit && result->submesh == GizmoAxis::Y))
    {
        mouseOnTop = true;
        if (state != Active)
        {
            GizmoAxis y;
            y.axis = GizmoAxis::Y;
            y.material = mouseDown ? cAxisWhite : cAxisGreenHi;
            y.ray = yRay;
            hits.append(y);
        }
    }

    zRay.ClosestPoint(mouseRay, &offsetZ);
    if ((distanceZ < cClickDistanceThreshold && offsetZ > 0 && offsetZ < cClickOffsetThreshold) ||
        (hit && result->submesh == GizmoAxis::Z))
    {
        mouseOnTop = true;
        if (state != Active)
        {
            GizmoAxis z;
            z.axis = GizmoAxis::Z;
            z.material = mouseDown ? cAxisWhite : cAxisBlueHi;
            z.ray = zRay;
            hits.append(z);
        }
    }

    AssetReferenceList materials;
    materials.Append(cAxisGreen);
    materials.Append(cAxisRed);
    materials.Append(cAxisBlue);

//    LogInfo("distance x " + ToString(distanceX));
//    LogInfo("distance y " + ToString(distanceY));
//    LogInfo("distance z " + ToString(distanceZ));

    switch(state)
    {
    case Inactive:
        //LogInfo("Inactive");
        input->ClearMouseCursorOverride();
        if (activeAxes.size())
            activeAxes.clear();
        if (mouseOnTop)
            state = Hovering; // fall through to Hovering.
        else
            break;
    case Hovering:
        e->Suppress();
        //LogInfo("Hovering");
        if (mouseOnTop)
        {
            foreach(const GizmoAxis &a, hits)
                materials.Set(a.axis, a.material);

            input->SetMouseCursorOverride(QCursor(Qt::OpenHandCursor));
            if (mouseDown) // fall trough to Active case.
                state = Active;
            else
                break;
        }
        else
        {
            state = Inactive;
            break;
        }
    case Active:
        e->Suppress();
        //LogInfo("Active");
        // If mouse released, fall through to default case.
        if (mouseDown)
        {
            if (hits.size())
            {
                activeAxes = hits;
                if (activeAxes.size() == 1)
                    prevPoint = activeAxes.first().ray.ClosestPoint(mouseRay);
                else if (activeAxes.size() == 2)
                {
                    float3 planeNormal = activeAxes[0].ray.dir.Cross(activeAxes[1].ray.dir).Normalized();
                    Plane plane(activeAxes[0].ray.pos, planeNormal);
                    float distance = 0.f;
                    if (plane.Intersects(mouseRay, &distance))
                        prevPoint = mouseRay.GetPoint(distance);
                }
//                else if (activeAxes.size() == 3 && gizmoType == EC_TransformGizmo::Scale)
//                    prevPoint = activeAxes.first().ray.ClosestPoint(mouseRay);
            }

            foreach(const GizmoAxis &a, activeAxes)
                materials.Set(a.axis, a.material);
            input->SetMouseCursorOverride(QCursor(Qt::ClosedHandCursor));
            // Emit only when dragging
            if (mouseDrag)
            {
                if (activeAxes.size() == 1)
                {
                    curPoint = activeAxes.first().ray.ClosestPoint(mouseRay);
                }
                else if (activeAxes.size() == 2)
                {
                    float3 planeNormal = activeAxes[0].ray.dir.Cross(activeAxes[1].ray.dir).Normalized();
                    Plane plane(activeAxes[0].ray.pos, planeNormal);
                    float distance = 0.f;
                    if (plane.Intersects(mouseRay, &distance))
                        curPoint = mouseRay.GetPoint(distance);
                }
//                else if (activeAxes.size() == 3 && gizmoType == EC_TransformGizmo::Scale)
//                    curPoint = activeAxes.first().ray.ClosestPoint(mouseRay);

                //std::stringstream ss;
                switch(gizmoType)
                {
                case EC_TransformGizmo::Translate:
                    //ss << curPoint-prevPoint;
                    //LogInfo("Emitting Translated(" + ss.str() + ")");
                    emit Translated(curPoint-prevPoint);
                    break;
                case EC_TransformGizmo::Rotate:
                    if (activeAxes.size() == 1)
                    {
                        float3 delta = curPoint - prevPoint;
                        const float sensitivity = 0.2f; // Apply a hardcoded sensitivity factor to the mouse delta values.
                        Quat q = Quat::RotateAxisAngle(activeAxes[0].ray.dir, sensitivity * delta[delta.Abs().MaxElementIndex()]);
                        emit Rotated(q);
                    }
                    break;
                case EC_TransformGizmo::Scale:
                    //ss << curPoint-prevPoint;
                    //LogInfo("Emitting Scaled(" + ss.str() + ")");
                    if (input && input->IsKeyDown(Qt::Key_Shift))
                        emit Scaled(curPoint-prevPoint);
                    else
                    {
                        float3 scale = curPoint-prevPoint;
                        emit Scaled(float3::FromScalar(scale.AverageOfElements()));
                    }
                    break;
                }
                prevPoint = curPoint;
            }

            break;
        }
    default:
        if (mouseOnTop)
        {
            foreach(const GizmoAxis &a, activeAxes)
                if (a.axis == 0)
                    materials.Set(0, cAxisGreenHi);
                else if (a.axis == 1)
                    materials.Set(1, cAxisRedHi);
                else if (a.axis == 2)
                    materials.Set(2, cAxisBlueHi);

            state = Hovering;
            input->SetMouseCursorOverride(QCursor(Qt::OpenHandCursor));
        }
        else
        {
            materials.Set(0, cAxisGreen);
            materials.Set(1, cAxisRed);
            materials.Set(2, cAxisBlue);

            state = Inactive;
            input->ClearMouseCursorOverride();
        }
        break;
    }

    if (mesh->meshMaterial.Get() != materials)
        mesh->meshMaterial.Set(materials, AttributeChange::Default);
}

void EC_TransformGizmo::OnFrameUpdate(float /*dt*/)
{
    if (!mesh || ogreWorld.expired())
        return;
    PROFILE(EC_TransformGizmo_OnFrameUpdate);
    mesh->SetAdjustScale(float3::FromScalar(DesiredGizmoScale()));
}

float EC_TransformGizmo::DesiredGizmoScale()
{
    if (!mesh || ogreWorld.expired())
        return 1.f;
    OgreWorldPtr world = ogreWorld.lock();
    Entity *cam = world->Renderer()->MainCamera();
    if (!cam)
        return 1.f;
    shared_ptr<EC_Placeable> placeable = cam->GetComponent<EC_Placeable>();
    if (!placeable)
        return 1.f;
    if (!mesh->HasMesh())
        return 1.f;
    // We always keep the transform gizmo constant-sized in the view.
    float3 gizmoPos = mesh->LocalToWorld().TranslatePart();
    float3 cameraPos = placeable->LocalToWorld().TranslatePart();
    float distance = gizmoPos.Distance(cameraPos);
    return std::max(0.1f, 0.1f*distance);
}

/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_TransformGizmo.cpp
 *  @brief  Enables visual manipulators (gizmos) for Transform attributes.
 */

#include "EC_TransformGizmo.h"

#include "Framework.h"
#include "Entity.h"
#include "InputAPI.h"
#include "AssetAPI.h"
#include "EC_Placeable.h"
#include "EC_Mesh.h"
#include "LoggingFunctions.h"
#include "OgreRenderingModule.h"
#include "Renderer.h"
#include "EC_Camera.h"
#include "OgreConversionUtils.h"
#include "UiAPI.h"
#include "UiGraphicsView.h"
#include "FrameAPI.h"
//#include "PhysicsModule.h"
//#include "PhysicsUtils.h"

#include <OgreRay.h>
#include <Ogre.h>

/// This code is taken from http://paulbourke.net/geometry/lineline3d/ .
/** dmnop = (xm - xn)(xo - xp) + (ym - yn)(yo - yp) + (zm - zn)(zo - zp) */
float Dmnop(Ogre::Vector3 v[], int m, int n, int o, int p)
{
    return (v[m].x - v[n].x) * (v[o].x - v[p].x) + (v[m].y - v[n].y) * (v[o].y - v[p].y) + (v[m].z - v[n].z) * (v[o].z - v[p].z);
}

/// Returns the point on the ray @c a that is closest to the line @c b.
Ogre::Vector3 ClosestPointOnLineAToLineB(const Ogre::Ray &a, const Ogre::Ray &b, float *d)
{
    Ogre::Vector3 v[4];
    v[0] = a.getOrigin();
    v[1] = a.getOrigin() + a.getDirection();
    v[2] = b.getOrigin();
    v[3] = b.getOrigin() + b.getDirection();
    float mu = (Dmnop(v,0,2,3,2) * Dmnop(v,3,2,1,0) - Dmnop(v,0,2,1,0)*Dmnop(v,3,2,3,2)) / (Dmnop(v,1,0,1,0)*Dmnop(v,3,2,3,2) - Dmnop(v,3,2,1,0)*Dmnop(v,3,2,1,0));
    if (d) *d = mu;
    return v[0] + mu * (v[1] - v[0]);
}

float DistanceBetweenTwoLines(const Ogre::Ray &a, const Ogre::Ray &b)
{
    Ogre::Vector3 pA = ClosestPointOnLineAToLineB(a, b, 0);
    Ogre::Vector3 pB = ClosestPointOnLineAToLineB(b, a, 0);
    return pA.distance(pB);
}

/// Returns the point on the ray @c a that is closest to the point @b.
Ogre::Vector3 ClosestPointOnLineAToPointB(const Ogre::Ray &a, const Ogre::Vector3 &b)
{
    Ogre::Vector3 p1 = a.getOrigin();
    Ogre::Vector3 p2 = a.getOrigin() + a.getDirection();
    Ogre::Vector3 p3 = b;
    float u = ((p3.x - p1.x)*(p2.x - p1.x) + (p3.y - p1.y)*(p2.y - p1.y) + (p3.z - p1.z)*(p2.z - p1.z)) / (p2-p1).squaredLength();
    return p1 + u * (p2 - p1);
}

/// This is an overloaded function.
Ogre::Vector3 ClosestPointOnLineAToPointB(const Ogre::Vector3 &lineStart, const Ogre::Vector3 &lineEnd, const Ogre::Vector3 &b, float *u)
{
    Ogre::Vector3 p1 = lineStart;
    Ogre::Vector3 p2 = lineEnd;
    Ogre::Vector3 p3 = b;
    *u = ((p3.x - p1.x)*(p2.x - p1.x) + (p3.y - p1.y)*(p2.y - p1.y) + (p3.z - p1.z)*(p2.z - p1.z)) / (p2-p1).squaredLength();
    return p1 + *u * (p2 - p1);
}

const AssetReference cTranslate("axis1.mesh");
const AssetReference cRotate("rotate1.mesh");
const AssetReference cScale("scale1.mesh");

const AssetReference cAxisRed("AxisRed.material");
const AssetReference cAxisRedHi("AxisRedHi.material");
const AssetReference cAxisGreen("AxisGreen.material");
const AssetReference cAxisGreenHi("AxisGreenHi.material");
const AssetReference cAxisBlue("AxisBlue.material");
const AssetReference cAxisBlueHi("AxisBlueHi.material");
const AssetReference cAxisWhite("AxisWhite.material");

EC_TransformGizmo::EC_TransformGizmo(Framework *fw) :
    IComponent(fw),
    gizmoType(Translate),
    state(Inactive)
{
    connect(this, SIGNAL(ParentEntitySet()), SLOT(Initialize()));

//    connect(fw->Frame(), SIGNAL(Updated(float)), SLOT(DrawDebug()));
    QString uniqueName("EC_TransformGizmo_" + fw->Asset()->GenerateUniqueAssetName("",""));
    input = fw->Input()->RegisterInputContext(uniqueName, 100);
    connect(input.get(), SIGNAL(MouseEventReceived(MouseEvent *)), SLOT(HandleMouseEvent(MouseEvent *)));

    OgreRenderer::OgreRenderingModule *module = GetFramework()->GetModule<OgreRenderer::OgreRenderingModule>();
    if (module)
        renderer = module->GetRenderer();
}

EC_TransformGizmo::~EC_TransformGizmo()
{
    input.reset();
}

void EC_TransformGizmo::SetPosition(const Vector3df &pos)
{
    if (placeable)
        placeable->SetPosition(pos);
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
    assert(GetParentEntity());
    if (!GetParentEntity())
        return;

    GetParentEntity()->SetName("TransformGizmo");

    placeable = boost::dynamic_pointer_cast<EC_Placeable>(GetParentEntity()->CreateComponent(
        EC_Placeable::TypeNameStatic(), "TransformGizmoPlaceable", AttributeChange::Default, false));
    if (!placeable)
    {
        LogError("Could not create EC_Placeable for EC_TransformGizmo.");
        return;
    }

    placeable->SetTemporary(true);
//    placeable->selectionLayer.Set(0, AttributeChange::Default); // ignore raycast
    placeable->visible.Set(false, AttributeChange::Default);

    mesh = boost::dynamic_pointer_cast<EC_Mesh>(GetParentEntity()->CreateComponent(
        EC_Mesh::TypeNameStatic(), "TransformGizmoMesh", AttributeChange::Default, false));
    if (!mesh)
    {
        LogError("Could not create EC_Mesh for EC_TransformGizmo.");
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

const float cClickDistanceThreshold = 0.125f;
const float cClickOffsetThreshold = 2.5f;

void EC_TransformGizmo::HandleMouseEvent(MouseEvent *e)
{
    using namespace OgreRenderer;
    if (!mesh || !placeable)
        return;
    if (!IsVisible())
        return;
    if (renderer.expired())
        return;
    boost::shared_ptr<EC_Camera> cam = renderer.lock()->GetActiveCamera();
    if (!cam)
        return;

    bool mouseOnTop = false;
    bool mouseDown = e->IsLeftButtonDown();
    if (e->eventType == MouseEvent::MouseReleased && e->button == MouseEvent::LeftButton)
        mouseDown = false;
    bool mouseDrag = e->eventType == MouseEvent::MouseMove && mouseDown;

    Ogre::Ray xRay, yRay, zRay;
    // If moving, ignore gizmo's placeable
    if ((e->eventType == MouseEvent::MousePressed && e->button == MouseEvent::LeftButton) ||
        (e->eventType == MouseEvent::MouseMove && e->otherButtons == 0))
    {
        xRay = Ogre::Ray(ToOgreVector3(placeable->transform.Get().position), ToOgreVector3(placeable->GetOrientation()*Vector3df::UNIT_X));
        yRay = Ogre::Ray(ToOgreVector3(placeable->transform.Get().position), ToOgreVector3(placeable->GetOrientation()*Vector3df::UNIT_Y));
        zRay = Ogre::Ray(ToOgreVector3(placeable->transform.Get().position), ToOgreVector3(placeable->GetOrientation()*Vector3df::UNIT_Z));
    }

    float relX = (float)e->x/GetFramework()->Ui()->GraphicsView()->size().width();
    float relY = (float)e->y/GetFramework()->Ui()->GraphicsView()->size().height();
    Ogre::Ray mouseRay = cam->GetMouseRay(relX, relY);

    QList<GizmoAxis> hits;

    float distanceX = DistanceBetweenTwoLines(mouseRay, xRay);
    float distanceY = DistanceBetweenTwoLines(mouseRay, yRay);
    float distanceZ = DistanceBetweenTwoLines(mouseRay, zRay);

    bool hit = false;
    RaycastResult *result = renderer.lock()->Raycast(e->x, e->y);
    if (result->entity && result->entity->GetComponent<EC_TransformGizmo>().get() == this)
        hit = true;

    float offsetX, offsetY, offsetZ;
    ClosestPointOnLineAToLineB(xRay, mouseRay, &offsetX);
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

    ClosestPointOnLineAToLineB(yRay, mouseRay, &offsetY);
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

    ClosestPointOnLineAToLineB(zRay, mouseRay, &offsetZ);
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
            foreach(GizmoAxis a, hits)
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
                    prevPoint = ClosestPointOnLineAToLineB(activeAxes.first().ray, mouseRay, 0);
                if (activeAxes.size() == 2)
                {
                    Ogre::Vector3 planeNormal = activeAxes[0].ray.getDirection().crossProduct(activeAxes[1].ray.getDirection());
                    planeNormal.normalise();
                    Ogre::Plane plane(planeNormal, activeAxes[0].ray.getOrigin());
                    std::pair<bool, Ogre::Real> p = mouseRay.intersects(plane);
                    if (fabs(p.second) < 1e-5f && p.first == false)
                        ;
                    else
                        prevPoint = mouseRay.getPoint(p.second);
                }
            }

            foreach(GizmoAxis a, activeAxes)
                materials.Set(a.axis, a.material);
            input->SetMouseCursorOverride(QCursor(Qt::ClosedHandCursor));
            // Emit only when dragging
            if (mouseDrag)
            {
                if (activeAxes.size() == 1)
                {
                    curPoint = ClosestPointOnLineAToLineB(activeAxes.first().ray, mouseRay, 0);
                }
                else if (activeAxes.size() == 2)
                {
                    Ogre::Vector3 planeNormal = activeAxes[0].ray.getDirection().crossProduct(activeAxes[1].ray.getDirection());
                    planeNormal.normalise();
                    Ogre::Plane plane(planeNormal, activeAxes[0].ray.getOrigin());
                    std::pair<bool, Ogre::Real> p = mouseRay.intersects(plane);
                    if (fabs(p.second) < 1e-5f && p.first == false)
                        ;
                    else
                        curPoint = mouseRay.getPoint(p.second);
                }

                std::stringstream ss;
                switch(gizmoType)
                {
                case EC_TransformGizmo::Translate:
                    //ss << ToCoreVector(curPoint-prevPoint);
                    //LogInfo("Emitting Translated(" + ss.str() + ")");
                    emit Translated(ToCoreVector(curPoint-prevPoint));
                    prevPoint = curPoint;
                    break;
                case EC_TransformGizmo::Rotate:
//                        LogInfo("Emitting Rotated(" + ss.str() + ")");
                        emit Rotated(Quaternion());
                    break;
                case EC_TransformGizmo::Scale:
                    //ss << ToCoreVector(curPoint-prevPoint);
                    //LogInfo("Emitting Scaled(" + ss.str() + ")");
                    emit Scaled(Vector3df(ToCoreVector(curPoint-prevPoint)));
                    prevPoint = curPoint;
                    break;
                }
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
                    materials.Set(1,cAxisRedHi);
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

    mesh->meshMaterial.Set(materials, AttributeChange::Default);
}

/*
btVector3 OgreToBt(const Ogre::Vector3 &v)
{
    return btVector3(v.x, v.y, v.z);
}

void EC_TransformGizmo::DrawDebug()
{
    Ogre::Vector3 xStart = xRay.getOrigin();
    Ogre::Vector3 xEnd = xRay.getOrigin()+300.f*xRay.getDirection();
    framework_->GetModule<Physics::PhysicsModule>()->drawLine(OgreToBt(xStart), OgreToBt(xEnd), btVector3(1,0,0));

    Ogre::Vector3 yStart = yRay.getOrigin();
    Ogre::Vector3 yEnd = yRay.getOrigin()+300.f*yRay.getDirection();
    framework_->GetModule<Physics::PhysicsModule>()->drawLine(OgreToBt(yStart), OgreToBt(yEnd), btVector3(0,1,0));

    Ogre::Vector3 zStart = zRay.getOrigin();
    Ogre::Vector3 zEnd = zRay.getOrigin()+300.f*zRay.getDirection();
    framework_->GetModule<Physics::PhysicsModule>()->drawLine(OgreToBt(zStart), OgreToBt(zEnd), btVector3(0,0,1));

    Ogre::Vector3 mouseStart = mouseRay.getOrigin();
    Ogre::Vector3 mouseEnd = mouseRay.getOrigin()+300.f*mouseRay.getDirection();
    framework_->GetModule<Physics::PhysicsModule>()->drawLine(OgreToBt(mouseStart), OgreToBt(mouseEnd), btVector3(1,1,1));
}
*/

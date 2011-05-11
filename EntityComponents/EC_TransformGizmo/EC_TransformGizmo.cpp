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

EC_TransformGizmo::EC_TransformGizmo(Framework *fw) :
    IComponent(fw)
{
    connect(this, SIGNAL(ParentEntitySet()), SLOT(Initialize()));

    QString uniqueName("EC_TransformGizmo_" + fw->Asset()->GenerateUniqueAssetName("",""));
    input = fw->Input()->RegisterInputContext(uniqueName, 100);
    connect(input.get(), SIGNAL(KeyEventReceived(KeyEvent *)), SLOT(HandleKeyEvent(KeyEvent *)));
    connect(input.get(), SIGNAL(MouseEventReceived(MouseEvent *)), SLOT(HandleMouseEvent(MouseEvent *)));
}

EC_TransformGizmo::~EC_TransformGizmo()
{
    input.reset();
}

void EC_TransformGizmo::Show()
{
    if (placeable)
        placeable->Show();
}

void EC_TransformGizmo::Hide()
{
    if (placeable)
        placeable->Hide();
}

void EC_TransformGizmo::SetVisible(bool visible)
{
    if (placeable)
    {
        if (visible)
            placeable->Show();
        else
            placeable->Hide();
    }
}

/** This code is taken from http://paulbourke.net/geometry/lineline3d/ .
    dmnop = (xm - xn)(xo - xp) + (ym - yn)(yo - yp) + (zm - zn)(zo - zp)
*/
/*
float Dmnop(Vector3df[] v, int m, int n, int o, int p)
{
    return (v[m].x - v[n].x) * (v[o].x - v[p].x) + (v[m].y - v[n].y) * (v[o].y - v[p].y) + (v[m].z - v[n].z) * (v[o].z - v[p].z);
}

///  Returns the point on the ray A that is closest to the line B.
Vector3df ClosestPointOnLineAToLineB(Ogre::Ray a, Ogre::Ray b)
{
    Vector3df[] v = new Vector3[4] { a.origin, a.origin + a.direction, b.origin, b.origin + b.direction };
    float mu = (Dmnop(v,0,2,3,2) * Dmnop(v,3,2,1,0) - Dmnop(v,0,2,1,0)*Dmnop(v,3,2,3,2)) / (Dmnop(v,1,0,1,0)*Dmnop(v,3,2,3,2) - Dmnop(v,3,2,1,0)*Dmnop(v,3,2,1,0));
    return v[0] + mu * (v[1] - v[0]);
}
*/
void EC_TransformGizmo::Initialize()
{
    placeable = boost::dynamic_pointer_cast<EC_Placeable>(GetParentEntity()->CreateComponent(
        EC_Placeable::TypeNameStatic(), "TransformGizmoPlaceable", AttributeChange::Default, false));
    if (!placeable)
    {
        LogError("");
        return;
    }

    placeable->SetTemporary(true);

    mesh = boost::dynamic_pointer_cast<EC_Mesh>(GetParentEntity()->CreateComponent(
        EC_Mesh::TypeNameStatic(), "TransformGizmoMesh", AttributeChange::Default, false));
    if (!mesh)
    {
        LogError("");
        return;
    }

    mesh->SetTemporary(true);

    //"axis1.material"
    //"axis1.mesh"
    //"rotate1.mesh"
    //"scale1.mesh"
    mesh->meshRef.Set(AssetReference("axis1.mesh"), AttributeChange::Default);
    AssetReferenceList materials;
    materials.Append(AssetReference("axis1.material"));
    mesh->meshMaterial.Set(materials, AttributeChange::Default);

    placeable->Show();
}

void EC_TransformGizmo::HandleKeyEvent(KeyEvent *e)
{
}

void EC_TransformGizmo::HandleMouseEvent(MouseEvent *e)
{
}

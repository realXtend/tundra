/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   TransformEditor.cpp
 *  @brief  Controls EC_TransformGizmos.
 */

#include "StableHeaders.h"
#include "TransformEditor.h"

#include "SceneManager.h"
#include "Entity.h"

#ifdef EC_TransformGizmo_ENABLED
#include "EC_TransformGizmo.h"
#endif

TransformEditor::TransformEditor()
{
}

void TransformEditor::SetSelectedEntities(const QList<EntityPtr> &entities)
{
    // gather EC_Placeables and their transform attributes to targets list
}

void TransformEditor::FocusGizmoPivotToAabbBottomCenter()
{
}

void TransformEditor::CreateGizmo()
{
    //SceneManager::CreateEntity(SceneManager::GetNextFreeIdLocal())
}

void TransformEditor::TranslateTargets(const Vector3df offset)
{
}

void TransformEditor::RotateTargets(const Quaternion &delta)
{
}

void TransformEditor::ScaleTargets(const Vector3df &offset)
{
}

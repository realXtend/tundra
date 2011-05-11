/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_TransformGizmo.cpp
 *  @brief  Enables visual manipulators (gizmos) for Transform attributes.
 */

#include "EC_TransformGizmo.h"

#include "Framework.h"
#include "InputAPI.h"
#include "AssetAPI.h"

EC_TransformGizmo::EC_TransformGizmo(Framework *fw) :
    IComponent(fw)
{
    QString uniqueName("EC_TransformGizmo_" + fw->Asset()->GenerateUniqueAssetName("",""));
    input = fw->Input()->RegisterInputContext(uniqueName, 100);
    connect(input.get(), SIGNAL(KeyEventReceived(KeyEvent *)), SLOT(HandleKeyEvent(KeyEvent *)));
    connect(input.get(), SIGNAL(MouseEventReceived(MouseEvent *)), SLOT(HandleMouseEvent(MouseEvent *)));
}

EC_TransformGizmo::~EC_TransformGizmo()
{
    input.reset();
}

void EC_TransformGizmo::HandleKeyEvent(KeyEvent *e)
{
}

void EC_TransformGizmo::HandleMouseEvent(MouseEvent *e)
{
}

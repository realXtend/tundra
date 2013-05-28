// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#define MATH_BULLET_INTEROP

// If PCH is disabled, leave the contents of this whole file empty to avoid any compilation unit getting any unnecessary headers.
#ifdef PCH_ENABLED
#include "CoreDefines.h"
#include "Framework.h"

// Disable unreferenced formal parameter coming from Bullet
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4100)
#endif
#include <btBulletDynamicsCommon.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#include <btBulletCollisionCommon.h>
#endif

// For conditions of distribution and use, see copyright notice in license.txt
///\file ServiceGetter.cpp
///\todo This file was used for a keyboard bindings configuration widget. It is no longer used,
///      and will be removed in the future.

#include "StableHeaders.h"
#include "ServiceGetter.h"
#include "Input.h"

namespace CoreUi
{
    ServiceGetter::ServiceGetter(Foundation::Framework *framework, QObject *parent) :
        QObject(parent),
        framework_(framework)
    {

    }

    void ServiceGetter::GetKeyBindings()
    {
    }

    void ServiceGetter::RestoreKeyBindings()
    {
    }
}

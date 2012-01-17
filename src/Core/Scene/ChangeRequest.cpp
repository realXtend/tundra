// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "ChangeRequest.h"

ChangeRequest::ChangeRequest() :
    allowed(true)
{
}

void ChangeRequest::Deny()
{
    allowed = false;
}

bool ChangeRequest::IsAllowed()
{
    return allowed;
}

void ChangeRequest::SetAllowed(bool allow)
{
    allowed = allow;
}

/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   ChangeRequest.h
    @brief  A result object to get return values from Qt signal handlers in the permission system (AboutToModifyEntity etc) */

#pragma once

#include "TundraCoreApi.h"

#include <QObject>

/// A result object to get return values from Qt signal handlers in the permission system (AboutToModifyEntity etc)
class TUNDRACORE_API ChangeRequest : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool allowed READ IsAllowed WRITE SetAllowed)

public:
    ChangeRequest();

    bool allowed;

public slots:
    bool IsAllowed();
    void SetAllowed(bool allow);
    void Deny();
};

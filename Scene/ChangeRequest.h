/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   ChangeRequest.h
 *  @brief  A result object to get return values from qt signal handlers in the permission system (AboutToModifyEntity etc)
 */

#ifndef incl_SceneManager_ChangeRequest_h
#define incl_SceneManager_ChangeRequest_h

#include <QObject>

class ChangeRequest : public QObject
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

#endif

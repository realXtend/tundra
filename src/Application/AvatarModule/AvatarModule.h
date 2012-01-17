// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "IModule.h"
#include "AvatarModuleApi.h"

#include <QPointer>

class AvatarEditor;

/// Provides EC_Avatar.
class AV_MODULE_API AvatarModule : public IModule
{
    Q_OBJECT

public:
    AvatarModule();
    virtual ~AvatarModule();

    void Load();
    void Initialize();

public slots:
    AvatarEditor* GetAvatarEditor() const;

    /// Start editing a specific entity's avatar
    void EditAvatar(const QString &entityName);

private:
    QPointer<AvatarEditor> avatarEditor;
};

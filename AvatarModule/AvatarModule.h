// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_AvatarModule_h
#define incl_AvatarModule_h

#include "IModule.h"

#include "AvatarModuleApi.h"
#include "InputFwd.h"
#include "SceneFwd.h"

#include <QObject>
#include <QList>
#include <QMap>

namespace Avatar
{
    class AvatarSceneManager;
    class AvatarHandler;
    class AvatarControllable;
    class AvatarEditor;

    typedef boost::shared_ptr<AvatarHandler> AvatarHandlerPtr;
    typedef boost::shared_ptr<AvatarControllable> AvatarControllablePtr;
    typedef boost::shared_ptr<AvatarEditor> AvatarEditorPtr;

    class AV_MODULE_API AvatarModule : public IModule
    {
        Q_OBJECT

    public:
        AvatarModule();
        virtual ~AvatarModule();

        void Load();
        void Initialize();
        void PostInitialize();
        void Uninitialize();
        void Update(f64 frametime);

    public slots:

        AvatarHandlerPtr GetAvatarHandler() { return avatar_handler_; }
        AvatarEditorPtr GetAvatarEditor() { return avatar_editor_; }
        AvatarControllablePtr GetAvatarControllable() { return avatar_controllable_; }

        /// Console command: start editing a specific entity's avatar
        void EditAvatar(const QString &entityName);

    private slots:
        /// Handle our key context input
        void KeyPressed(KeyEvent *key);
        void KeyReleased(KeyEvent *key);

    private:
        /// AvatarModules input context
        InputContextPtr avatar_context_;

        AvatarHandlerPtr avatar_handler_;
        AvatarControllablePtr avatar_controllable_;
        AvatarEditorPtr avatar_editor_;
    };
}

#endif

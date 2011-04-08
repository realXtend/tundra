// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Avatar_AvatarEditor_h
#define incl_Avatar_AvatarEditor_h

#ifdef ENABLE_TAIGA_SUPPORT

#include "AvatarModule.h"
#include "AvatarModuleApi.h"

#include "ui_avatareditor.h"

#include <QWidget>

class QTabWidget;
class EC_Avatar;
class AvatarDescAsset;

namespace Avatar
{
    /// Avatar editing window. Owned by RexLogicModule.
    class AV_MODULE_API AvatarEditor : public QWidget, public Ui::AvatarEditor
    {
        Q_OBJECT

    public:
        explicit AvatarEditor(AvatarModule *avatar_module);
        ~AvatarEditor();

    public slots:
        /// Rebuild edit view
        void RebuildEditView();
        
        /// Export click handler
        void ExportAvatar();

        /// Local export click handler
        void ExportAvatarLocal();

        /// New avatar click handler
        void LoadAvatar();

        /// Revert click handler
        void RevertAvatar();

        /// Change texture click handler
        void ChangeTexture();

        /// New attachment click handler
        void AddAttachment();

        /// Remove attachment click handler
        void RemoveAttachment();

        /// Bone modifier scrollbar value handler
        void BoneModifierValueChanged(int value);

        /// Morph modifier scrollbar value handler
        void MorphModifierValueChanged(int value);

        /// Master appearance modifier scrollbar value handler
        void MasterModifierValueChanged(int value);

        //! Set avatar entity by name to edit
        void SetAvatarEntityName(QString name);
        
    protected:
        /// QWidget override.
        void changeEvent(QEvent* e);

signals:
        void EditorStatus(const QString &message, int timeout = 7000);
        void EditorError(const QString &message, int timeout = 7000);
        void EditorHideMessages();

    private:
        /// Owner module.
        AvatarModule *avatar_module_;

        /// Create editor window
        void InitEditorWindow();
        
        //! Get the avatar entity to edit. If avatar_entity_name_ is empty, try to get the user's avatar from AvatarHandler.
        Scene::EntityPtr GetAvatarEntity();
        //! Get the avatar entity, avatar component, and avatar description. If all are non-null, return true
        bool GetAvatarDesc(Scene::EntityPtr& entity, EC_Avatar*& avatar, AvatarDescAsset*& desc);

        /// Clear a panel
        void ClearPanel(QWidget* panel);

        /// Create or get a tabbed scrollarea panel
        QWidget* GetOrCreateTabScrollArea(QTabWidget* tabs, const std::string& name);

        /// Ask a filename from the user. Store the directory used.
        std::string GetOpenFileName(const std::string& filter, const std::string& prompt);

        /// Ask a filename from the user for saving. Store the directory used.
        std::string GetSaveFileName(const std::string& filter, const std::string& prompt);

        /// Last used directory for selecting avatars, attachments, textures
        std::string last_directory_;

        //! Avatar entity to edit. If empty, try to get the user's avatar from AvatarHandler.
        QString avatar_entity_name_;

        bool reverting_;
    };
}

#endif

#endif

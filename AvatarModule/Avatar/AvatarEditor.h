// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLogic_AvatarEditor_h
#define incl_RexLogic_AvatarEditor_h

#include "AvatarModule.h"
#include "AvatarModuleApi.h"

#include <QWidget>

class QTabWidget;

namespace AvatarModule
{
    //! Avatar editing window. Owned by RexLogicModule.
    class AV_MODULE_API AvatarEditor : public QWidget
    {
        Q_OBJECT

    public:
        explicit AvatarEditor(AvatarModule *avatar_module);
        ~AvatarEditor();

        //! Rebuild edit view
        void RebuildEditView();

    public slots:
        //! Export click handler
        void ExportAvatar();

        //! Local export click handler
        void ExportAvatarLocal();

        //! New avatar click handler
        void LoadAvatar();

        //! Revert click handler
        void RevertAvatar();

        //! Change texture click handler
        void ChangeTexture();

        //! New attachment click handler
        void AddAttachment();

        //! Remove attachment click handler
        void RemoveAttachment();

        //! Bone modifier scrollbar value handler
        void BoneModifierValueChanged(int value);

        //! Morph modifier scrollbar value handler
        void MorphModifierValueChanged(int value);

        //! Master appearance modifier scrollbar value handler
        void MasterModifierValueChanged(int value);

    protected:
        /// QWidget override.
        void changeEvent(QEvent* e);

    private:
        //! Owner module.
        AvatarModule *avatar_module_;

        //! Create editor window
        void InitEditorWindow();

        //! Clear a panel
        void ClearPanel(QWidget* panel);

        //! Create or get a tabbed scrollarea panel
        QWidget* GetOrCreateTabScrollArea(QTabWidget* tabs, const std::string& name);

        //! Ask a filename from the user. Store the directory used.
        std::string GetOpenFileName(const std::string& filter, const std::string& prompt);

        //! Ask a filename from the user for saving. Store the directory used.
        std::string GetSaveFileName(const std::string& filter, const std::string& prompt);

        //! Main widget for avatar editor
        QWidget *avatar_widget_;

        //! Last used directory for selecting avatars, attachments, textures
        std::string last_directory_;
    };
}

#endif

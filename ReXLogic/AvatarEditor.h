// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLogic_AvatarEditor_h
#define incl_RexLogic_AvatarEditor_h

#include "Foundation.h"
#include "UICanvas.h"

#include <QObject>

class QTabWidget;
    
namespace RexLogic
{
    class RexLogicModule;
    
    //! Avatar editing window. Owned by RexLogicModule.
    class AvatarEditor : public QObject
    {
        Q_OBJECT
        
    public:
        AvatarEditor(RexLogicModule *rexlogicmodule);
        ~AvatarEditor();
        
        //! Toggle visibility of editor window
        void Toggle();
        //! Rebuild edit view
        void RebuildEditView();
        
    public slots:
        //! Export click handler
        void ExportAvatar();
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
        //! Scrollbar quantizer handler (to try to prevent internal mode clipping bug)
        void QuantizeScrollBar(int value);
                
    private:
        RexLogicModule *rexlogicmodule_;
        
        //! Create editor window
        void InitEditorWindow();
        
        //! Clear a panel
        void ClearPanel(QWidget* panel);
 
        //! Create or get a tabbed scrollarea panel
        QWidget* GetOrCreateTabScrollArea(QTabWidget* tabs, const std::string& name);
        
        //! Ask a filename from the user. Store the directory used.
        std::string GetOpenFileName(const std::string& filter, const std::string& prompt);
               
        //! Canvas for avatar editor
        boost::shared_ptr<QtUI::UICanvas> canvas_;
        
        //! Main widget for avatar editor
        QWidget *avatar_widget_;
        
        //! Last used directory for selecting avatars, attachments, textures
        std::string last_directory_;
    };
}

#endif

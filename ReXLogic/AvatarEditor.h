// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLogic_AvatarEditor_h
#define incl_RexLogic_AvatarEditor_h

#include "Foundation.h"
#include "UICanvas.h"

#include <QObject>

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
        //! Rebuild edit view, called when user avatar has changed/reloaded
        void RebuildEditView();
        
    public slots:
        //! Export button press handler
        void ExportAvatar();
        //! Close button press handler
        void Close();
        //! Load button press handler
        void LoadAvatar();
        //! Revert button press handler
        void RevertAvatar();
        //! Change texture press handler
        void ChangeTexture();
        //! Morph scrollbar value handler
        void MorphValueChanged(int value);
        
    private:
        RexLogicModule *rexlogicmodule_;
        
        //! Create editor window
        void InitEditorWindow();
        
        //! Clear a panel
        void ClearPanel(QWidget* panel);
        
        //! Canvas for avatar editor
        boost::shared_ptr<QtUI::UICanvas> canvas_;
        
        //! Main widget for avatar editor
        QWidget *avatar_widget_;
    };
}

#endif

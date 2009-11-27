// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_QtModule_UICanvasTestEdit_h
#define incl_QtModule_UICanvasTestEdit_h

#include "Foundation.h"
#include "UICanvas.h"

#include <QObject>

namespace QtUI
{
    //! An editor view for assigning EC_UICanvases to world objects, for test purposes
    /*! Note: QtModule is not the final placement for this functionality, will eventually be moved
     */
    class UICanvasTestEdit : public QObject
    {
        Q_OBJECT
        
    public:
        UICanvasTestEdit(Foundation::Framework* framework);
        ~UICanvasTestEdit();
        
        //! Set new entity to use
        void SetEntityId(Core::entity_id_t entity_id);
        
    public slots:
        //! Handle UICanvas bind to entity -action
        void BindCanvas();
        //! Handle UICanvas unbind from entity -action
        void UnbindCanvas();
        //! Handle show of editor
        void Shown();
        //! Handle hide of editor
        void Hidden();
        
    private:
        //! Initialize the canvas/widget
        void InitEditorWindow();
        
        //! Refresh entity subentity combo box
        void RefreshSubmeshes();
        //! Refresh canvas combo box
        void RefreshCanvases();
        
        //! Framework
        Foundation::Framework* framework_;
        
        //! Canvas for the editor view
        boost::shared_ptr<QtUI::UICanvas> canvas_;
        
        //! Main widget for the editor view
        QWidget *editor_widget_;        

        //! Last selected entity
        Core::entity_id_t last_entity_id_;
    };
}

#endif
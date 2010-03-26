// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_QtModule_UICanvasTestEdit_h
#define incl_QtModule_UICanvasTestEdit_h

#include "Foundation.h"
#include "UICanvas.h"

#include <QObject>

namespace UiServices
{
    class UiProxyWidget;
}

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
        void SetEntityId(entity_id_t entity_id);
        
    public slots:
        //! Handle UICanvas bind to entity -action
        void BindCanvas();
        //! Handle UICanvas unbind from entity -action
        void UnbindCanvas();
        //! Handle show/hide of editor
        void Shown(bool visible);
        
    private:
        //! Initialize the canvas/widget
        void InitEditorWindow();
        
        //! Refresh entity subentity combo box
        void RefreshSubmeshes();
        //! Refresh canvas combo box
        void RefreshCanvases();
        
        //! Framework
        Foundation::Framework* framework_;
        
        //! Main widget for the editor view
        QWidget *editor_widget_;        

        //! Last selected entity
        entity_id_t last_entity_id_;

        //! Ui services proxy widget for ui
        UiServices::UiProxyWidget *editor_widget_proxy_;

        //! Container for all created UICanvases
        QList<boost::shared_ptr<QtUI::UICanvas> > proxy_widget_canvases_;

    };
}

#endif
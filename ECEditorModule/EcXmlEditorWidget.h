/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EcXmlEditorWidget.h
 *  @brief  Entity-component XML editor widget used for editing EC attributes in XML format.
 */

#ifndef incl_ECEditorModule_EcXmlEditorWidget_h
#define incl_ECEditorModule_EcXmlEditorWidget_h

#include "ForwardDefines.h"

#include <QWidget>

namespace Foundation
{
    class Framework;
}

class QTextEdit;

namespace ECEditor
{
    //! \ingroup ECEditorModuleClient.
    class EcXmlEditorWidget : public QWidget
    {
        Q_OBJECT

    public:
        /// Constructor.
        /// @param framework Framework.
        /// @param parent Parent widget.
        EcXmlEditorWidget(Foundation::Framework *framework, QWidget *parent = 0);

        /// Destructor.
        ~EcXmlEditorWidget();

        /// Sets the entity which EC attributes are shown as XML.
        /// @param entity Entity.
        void SetEntity(Scene::EntityPtr entity);

        /// Sets the component which EC attributes are shown as XML.
        /// @param component Component.
        void SetComponent(Foundation::ComponentPtr component);

    public slots:
        /// Refreshes XML data.
        void Refresh();

        /// Reverts the modifications.
        void Revert();

        /// Saves the modifications.
        void Save();

    protected:
        /// QWidget override.
        void changeEvent(QEvent *event);

    private:
        /// Framework.
        Foundation::Framework *framework_;

        /// XML text edit field.
        QTextEdit *xmlEdit_;

        /// Entity whom EC('s) we're editing.
        Scene::EntityWeakPtr entity_;

        /// Component which we're editing if we're editing single component, not entire entity.
        Foundation::ComponentWeakPtr component_;
    };
}

#endif

/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EcXmlEditorWidget.h
 *  @brief  Entity-component XML editor widget used for editing EC attributes in XML format.
 */

#ifndef incl_ECEditorModule_EcXmlEditorWidget_h
#define incl_ECEditorModule_EcXmlEditorWidget_h

#include "ForwardDefines.h"
#include "SceneFwd.h"

#include <QWidget>

class QTextEdit;

/// Entity-component XML editor widget used for editing EC attributes in XML format.
/// \ingroup ECEditorModuleClient.
class EcXmlEditorWidget : public QWidget
{
    Q_OBJECT

public:
    /// Constructor.
    /// @param framework Framework.
    /// @param parent Parent widget.
    EcXmlEditorWidget(Framework *framework, QWidget *parent = 0);

    /// Destructor.
    ~EcXmlEditorWidget();

    /// Sets the entities whose EC attributes we want to edit as XML.
    /// @param entities List of entities.
    void SetEntity(const QList<EntityPtr> &entities);

    /// Sets the components whose EC attributes we want to edit as XML.
    /// @param components List of components.
    void SetComponent(const QList<ComponentPtr> &components);

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
    Framework *framework_;

    /// XML text edit field.
    QTextEdit *xmlEdit_;

    /// Entities whose EC's we're editing.
    QList<EntityWeakPtr> entities_;

    /// Components which we're editing.
    QList<ComponentWeakPtr > components_;
};

#endif

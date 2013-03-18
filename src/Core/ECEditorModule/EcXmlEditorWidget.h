/**
 *  For conditions of distribution and use, see copyright notice in LICENSE
 *
 *  @file   EcXmlEditorWidget.h
 *  @brief  Widget for entity-component XML editing.
 */

#pragma once

#include "SceneFwd.h"

#include <QWidget>
#include <QDomDocument>

class QTextEdit;

class Framework;

/// Widget for entity-component XML editing.
/** \ingroup ECEditorModuleClient. */
class EcXmlEditorWidget : public QWidget
{
    Q_OBJECT

public:
    /// Constructor.
    /** @param fw Framework.
        @param parent Parent widget. */
    EcXmlEditorWidget(Framework *fw, QWidget *parent = 0);

    ~EcXmlEditorWidget();

    /// Sets the entities whose EC attributes we want to edit as XML.
    /** @param entities List of entities. */
    void SetEntity(const QList<EntityPtr> &entities);

    /// Sets the components whose EC attributes we want to edit as XML.
    /** @param components List of components. */
    void SetComponent(const QList<ComponentPtr> &components);

public slots:
    /// Refreshes XML data.
    void Refresh();

    /// Reverts the modifications.
    void Revert();

    /// Saves the modifications.
    void Save();

signals:
    /// Emitted when changes to the entity-component XML are saved.
    void Saved();

protected:
    /// QWidget override.
    void changeEvent(QEvent *event);

private:
    Framework *framework; ///< Framework.
    QTextEdit *xmlEdit; ///< XML text edit field.
    QDomDocument unsavedState_;
    QList<EntityWeakPtr> targetEntities; ///< Entities whose EC's we're editing.
    QList<ComponentWeakPtr > targetComponents; ///< Components which we're editing.
};

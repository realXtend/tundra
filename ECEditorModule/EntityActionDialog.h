/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EntityActionDialog.h
 *  @brief  Dialog for invoking entity actions.
 */

#ifndef incl_ECEditorModule_EntityActionDialog_h
#define incl_ECEditorModule_EntityActionDialog_h

#include "ECEditorModuleApi.h"
#include "ForwardDefines.h"
#include "EntityAction.h"

#include <QDialog>
#include <QItemDelegate>

class QLineEdit;
class QComboBox;
class QCheckBox;

/// Dialog for invoking entity actions.
class ECEDITOR_MODULE_API EntityActionDialog : public QDialog
{
    Q_OBJECT

public:
    /// Constructs the dialog.
    /** Populates action combo box with union of all the actions of all the @c entities.
        The dialog is destroyed when hide() or close() is called for it.
        @param entities Entities for which action is performed.
        @param parent Parent widget.
        @param f Window flags.
    */
    EntityActionDialog(const QList<Scene::EntityWeakPtr> &entities, QWidget *parent = 0, Qt::WindowFlags f = 0);
    ~EntityActionDialog();

public slots:
    /// Returns list of entities for which the action is triggered.
    QList<Scene::EntityWeakPtr> Entities() const;

    /// Returns execution type(s) of the action.
    EntityAction::ExecutionTypeField ExecutionType() const;

    /// Returns name of the action.
    QString Action() const;

    /// Returns parameter list for the action.
    QStringList Parameters() const;

protected:
    /// QWidget override.
    void hideEvent(QHideEvent *);

private:
    /// Action name combo box.
    QComboBox *actionComboBox;

    /// Line edit for actions parameters.
    QLineEdit *parametersLineEdit;

    /// "Local" check box.
    QCheckBox *localCheckBox;

    /// "Server" check box.
    QCheckBox *serverComboBox;

    /// "Peers" check box.
    QCheckBox *peersComboBox;

    /// "Execute" button.
    QPushButton *execButton;

    /// "Execute and Close" button.
    QPushButton *execAndCloseButton;

    /// List of entities for 
    QList<Scene::EntityWeakPtr> entities;

private slots:
    /// Checks that we have some execution type checked. If not, "Execute" and "Execute and Close" buttons are disabled.
    void CheckExecuteAccepted();
};

#endif

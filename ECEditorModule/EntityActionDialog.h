/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EntityActionDialog.h
 *  @brief  
 */

#ifndef incl_ECEditorModule_EntityActionDialog_h
#define incl_ECEditorModule_EntityActionDialog_h

#include "ECEditorModuleApi.h"
#include "ForwardDefines.h"
#include "EntityAction.h"

#include <QDialog>

class QLineEdit;
class QComboBox;

///
class ECEDITOR_MODULE_API EntityActionDialog: public QDialog
{
    Q_OBJECT

public:
    /// Constructs the dialog. The dialog is destroyed when hide() or close() is called for it.
    /** @param entities Entities for which action is performed.
        @param act Name of the action.
        @param parent Parent widget.
        @param f Window flags.
    */
    EntityActionDialog(const QList<Scene::EntityWeakPtr> &entities, const QString &act, QWidget *parent = 0, Qt::WindowFlags f = 0);
    ~EntityActionDialog();

public slots:
    /// Returns list of entities for which the action is triggered.
    QList<Scene::EntityWeakPtr> Entities() const;

    /// Returns execution type of the action.
    EntityAction::ExecutionType ExecutionType() const;

    /// Returns name of the action.
    QString Action() const;

    /// Returns parameter list for the action.
    QStringList Parameters() const;

protected:
    /// 
    void hideEvent(QHideEvent *);

private:
    ///
    QString action;

    ///
    QLineEdit *parametersLineEdit;

    ///
    QComboBox *typeComboBox;

    /// List of entities for 
    QList<Scene::EntityWeakPtr> entities;
};

#endif

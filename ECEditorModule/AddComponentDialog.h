// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ECEditorModule_AddComponentDialog_h
#define incl_ECEditorModule_AddComponentDialog_h

#include "ECEditorModuleApi.h"
#include "CoreTypes.h"
#include "AttributeChangeType.h"

#include <QDialog>

class QLabel;
class QLineEdit;
class QComboBox;
class QCheckBox;
class QPushButton;

class Framework;

/// Dialog for adding new component to entity.
class ECEDITOR_MODULE_API AddComponentDialog: public QDialog
{
    Q_OBJECT

public:
    /// Constructs the dialog.
    /** \param fw Framework.
        \param ids IDs of entities to which the component will be added.
        \param parent Parent widget.
        \param f Window flags.
    */
    AddComponentDialog(Framework *fw, const QList<entity_id_t> &ids, QWidget *parent = 0, Qt::WindowFlags f = 0);

    /// Destroyes the dialog.
    ~AddComponentDialog();

    /// Sets available component types.
    void SetComponentList(const QStringList &component_types);

    /// Sets default name.
    void SetComponentName(const QString &name);

public slots:
    /// Returns component typename
    QString GetTypeName() const;

    /// Returns component name
    QString GetName() const;

    /// Returns if synchronization check box is checked or not.
    bool GetSynchronization() const;

    /// Returns if temporary check box is checked or not.
    bool GetTemporary() const;

    /// Returns entity IDs of the entities to which the component is added to.
    QList<entity_id_t> GetEntityIds() const;

private slots:
    /// Make sure that component name don't duplicate with existing entity's components, and if it do disable ok button.
    void CheckComponentName(const QString &name);

protected:
    /// Override event from QDialog.
    void hideEvent(QHideEvent *event);

private:
    QLineEdit *name_line_edit_;
    QComboBox *type_combo_box_;
    QCheckBox *sync_check_box_;
    QCheckBox *temp_check_box_;
    QPushButton *ok_button_;
    QPushButton *cancel_button_;
    typedef QList<entity_id_t> EntityIdList;
    EntityIdList entities_; ///< Entities for which the new component is planned to be added.
    Framework *framework_;
};

#endif

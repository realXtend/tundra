// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

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
class ECEDITOR_MODULE_API AddComponentDialog : public QDialog
{
    Q_OBJECT

public:
    /// Constructs the dialog.
    /** @param fw Framework.
        @param ids IDs of entities to which the component will be added.
        @param parent Parent widget.
        @param f Window flags. */
    AddComponentDialog(Framework *fw, const QList<entity_id_t> &ids, QWidget *parent = 0, Qt::WindowFlags f = 0);

    ~AddComponentDialog();

    /// Sets available component types.
    /** In order to improve readibility and usability, the "EC_" prefix is stripped from the type name.
        TypeName() however returns the full type name. */
    void SetComponentList(const QStringList &componentTypes);

    /// Sets default name.
    void SetComponentName(const QString &name);

public:
    /// Returns component typename, guaranteed to begin with the "EC_" prefix.
    QString TypeName() const;

    /// Returns component name
    QString Name() const;

    /// Returns if synchronization check box is checked or not.
    bool IsReplicated() const;

    /// Returns if temporary check box is checked or not.
    bool IsTemporary() const;

    /// Returns entity IDs of the entities to which the component is added to.
    QList<entity_id_t> EntityIds() const;

private slots:
    /// Make sure that component name don't duplicate with existing entity's components, and if it do disable ok button.
    void CheckComponentName();

    /// Check ui state and updates ui accordingly.
    void CheckTempAndSync();

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
    QLabel *errorLabel;
    QList<entity_id_t> entities_; ///< Entities for which the new component is planned to be added.
    Framework *framework_;
};

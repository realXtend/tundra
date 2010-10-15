// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ECEditorModule_AddComponentDialog_h
#define incl_ECEditorModule_AddComponentDialog_h

#include <QDialog>
#include "CoreTypes.h"
#include "AttributeChangeType.h"

class QLabel;
class QLineEdit;
class QComboBox;
class QPushButton;

namespace Foundation
{
    class Framework;
}

namespace ECEditor
{
    class AddComponentDialog: public QDialog
    {
        Q_OBJECT

    public:
        AddComponentDialog(Foundation::Framework *framework, entity_id_t entity_id, QWidget *parent = 0, Qt::WindowFlags f = 0);
        ~AddComponentDialog();

        //! Set list of available component types.
        void SetComponentList(const QStringList &component_types);

        //! Set default name.
        void SetComponentName(const QString &name);

    public slots:
        //! Get component typename
        QString GetTypename() const;

        //! Get component name
        QString GetName() const;

        //! Get synchronization mode from dialog. If combobox text is invalid return AttrbuteChange::Default.
        AttributeChange::Type GetSynchronization() const;

        //! Get the entity that component is added to.
        entity_id_t GetEntityId() const;

    private slots:
        //! Make sure that component name dont duplicate with existing entity's components, and if it do disable ok button.
        void CheckComponentName(const QString &name);

    private:
        void Initialize();

        QLabel *component_type_label_;
        QLabel *component_name_label_;
        QLabel *component_synch_label_;
        QLineEdit *name_line_edit_;
        QComboBox *type_combo_box_;
        QComboBox *synch_combo_box_;
        QPushButton *ok_button_;
        QPushButton *cancel_button_;

        //! Entity that new component is planned to be added.
        entity_id_t entity_id_;
        Foundation::Framework *framework_;
    };
}

#endif
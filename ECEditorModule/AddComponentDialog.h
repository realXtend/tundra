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
        AddComponentDialog(Foundation::Framework *framework, QList<entity_id_t> entities, QWidget *parent = 0, Qt::WindowFlags f = 0);
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
        QList<entity_id_t> GetEntityIds() const;

    private slots:
        //! Make sure that component name don't duplicate with existing entity's components, and if it do disable ok button.
        void CheckComponentName(const QString &name);

    protected:
        //! Over-ride event from QDialog.
        void hideEvent(QHideEvent *event);

    private:
        void Initialize();

        QLabel *component_count_label_;
        QLabel *component_type_label_;
        QLabel *component_name_label_;
        QLabel *component_synch_label_;
        QLineEdit *name_line_edit_;
        QComboBox *type_combo_box_;
        QComboBox *synch_combo_box_;
        QPushButton *ok_button_;
        QPushButton *cancel_button_;

        //! Entities that new component is planned to be added.
        typedef QList<entity_id_t> EntityIdList;
        EntityIdList entities_;
        Foundation::Framework *framework_;
    };
}

#endif
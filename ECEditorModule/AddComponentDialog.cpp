// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "AddComponentDialog.h"
#include "Framework.h"
#include "SceneManager.h"
#include "Entity.h"

#include <QLayout>
#include <QLineEdit>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>

#include "MemoryLeakCheck.h"

namespace ECEditor
{
    AddComponentDialog::AddComponentDialog(Foundation::Framework *fw, const QList<entity_id_t> &ids, QWidget *parent, Qt::WindowFlags f):
        QDialog(parent, f),
        framework_(fw),
        entities_(ids),
        component_type_label_(0),
        component_name_label_(0),
        name_line_edit_(0),
        type_combo_box_(0),
        synch_combo_box_(0),
        ok_button_(0),
        cancel_button_(0)
    {
        setAttribute(Qt::WA_DeleteOnClose);
        if (graphicsProxyWidget())
            graphicsProxyWidget()->setWindowTitle(tr("Add new component"));

        setWindowTitle(tr("Add new component"));

        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->setContentsMargins(5,5,5,5);
        layout->setSpacing(6);

        if(entities_.size() > 1)
        {
            component_count_label_ = new QLabel(QString::number(entities_.size()) + tr(" entities selected."), this);
            layout->addWidget(component_count_label_); 
        }

        component_type_label_ = new QLabel(tr("Component:"), this);
        component_name_label_ = new QLabel(tr("Name:"), this);
        component_synch_label_ = new QLabel(tr("Synchronization:"), this);
        name_line_edit_ = new QLineEdit(this);
        connect(name_line_edit_, SIGNAL(textChanged(const QString&)), this, SLOT(CheckComponentName(const QString&)));
        type_combo_box_ = new QComboBox(this);
        connect(type_combo_box_, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(CheckComponentName(const QString&)));
        synch_combo_box_ = new QComboBox(this);
        synch_combo_box_->addItem(tr("Replicate"));
        synch_combo_box_->addItem(tr("Local"));

        layout->addWidget(component_name_label_);
        layout->addWidget(name_line_edit_);
        layout->addWidget(component_type_label_);
        layout->addWidget(type_combo_box_);
        layout->addWidget(component_synch_label_);
        layout->addWidget(synch_combo_box_);

        QSpacerItem *spacer = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);
        layout->insertSpacerItem(-1, spacer);

        QHBoxLayout *buttons_layout = new QHBoxLayout();
        layout->insertLayout(-1, buttons_layout);
        QSpacerItem *button_spacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
        buttons_layout->addSpacerItem(button_spacer);

        ok_button_ = new QPushButton(tr("Ok"), this);
        cancel_button_ = new QPushButton(tr("Cancel"), this);

        connect(ok_button_, SIGNAL(clicked()), this, SLOT(accept()));
        connect(cancel_button_, SIGNAL(clicked()), this, SLOT(reject()));

        buttons_layout->addWidget(ok_button_);
        buttons_layout->addWidget(cancel_button_);
    }

    AddComponentDialog::~AddComponentDialog()
    {
    }

    void AddComponentDialog::SetComponentList(const QStringList &component_types)
    {
        type_combo_box_->addItems(component_types);
    }

    void AddComponentDialog::SetComponentName(const QString &name)
    {
        name_line_edit_->setText(name);
    }

    QString AddComponentDialog::GetTypename() const
    {
        return type_combo_box_->currentText();
    }

    QString AddComponentDialog::GetName() const
    {
        return name_line_edit_->text();
    }

    bool AddComponentDialog::GetSynchronization() const
    {
        QString synchText = synch_combo_box_->currentText();
        if(synchText == tr("Replicate"))
            return true;
        else if(synchText == tr("Local"))
            return true;

        return true;
    }

    QList<entity_id_t> AddComponentDialog::GetEntityIds() const
    {
        return entities_;
    }

    void AddComponentDialog::CheckComponentName(const QString &name)
    {
        bool name_dublicates = false;
        Scene::ScenePtr scene = framework_->GetDefaultWorldScene();
        if(scene && type_combo_box_ && name_line_edit_)
        {
            Scene::EntityPtr entity;
            for(uint i = 0; i < entities_.size(); i++)
            {
                entity = scene->GetEntity(entities_[i]);
                if(entity->HasComponent(type_combo_box_->currentText(), name_line_edit_->text()))
                {
                    name_dublicates = true;
                    break;
                }
            }
            if (name_dublicates)
                ok_button_->setEnabled(false);
            else
                ok_button_->setEnabled(true);
        }
    }

    void AddComponentDialog::hideEvent(QHideEvent *event)
    {
        deleteLater();
    }
}

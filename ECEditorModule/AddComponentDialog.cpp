// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "AddComponentDialog.h"
#include "ECEditorModule.h"
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
    AddComponentDialog::AddComponentDialog(Foundation::Framework *framework, entity_id_t entity_id, QWidget *parent, Qt::WindowFlags f):
        framework_(framework),
        QDialog(parent, f),
        entity_id_(entity_id),
        component_type_label_(0),
        component_name_label_(0),
        name_line_edit_(0),
        type_combo_box_(0),
        synch_combo_box_(0),
        ok_button_(0),
        cancel_button_(0)
    {
        Initialize();
    }

    AddComponentDialog::~AddComponentDialog()
    {
        
    }

    void AddComponentDialog::SetComponentList(const QStringList &component_types)
    {
        if(type_combo_box_)
            type_combo_box_->addItems(component_types);
        else
            ECEditorModule::LogWarning("Fail to add component list to component dialog, dialog wasn't initialized.");
    }

    void AddComponentDialog::SetComponentName(const QString &name)
    {
        if(name_line_edit_)
            name_line_edit_->setText(name);
        else
            ECEditorModule::LogWarning("Fail to add component name to component dialog, dialog wasn't initialized.");
    }

    QString AddComponentDialog::GetTypename() const
    {
        return type_combo_box_->currentText();
    }

    QString AddComponentDialog::GetName() const
    {
        return name_line_edit_->text();
    }

    AttributeChange::Type AddComponentDialog::GetSynchronization() const
    {
        if(synch_combo_box_)
        {
            QString synchText = synch_combo_box_->currentText();
            if(synchText == "Replicate")
                return AttributeChange::Replicate;
            else if(synchText == "Local")
                return AttributeChange::LocalOnly;
        }
        return AttributeChange::Default;
    }

    entity_id_t AddComponentDialog::GetEntityId() const
    {
        return entity_id_;
    }

    void AddComponentDialog::CheckComponentName(const QString &name)
    {
        Scene::ScenePtr scene = framework_->GetDefaultWorldScene();
        if(scene && type_combo_box_ && name_line_edit_)
        {
            Scene::EntityPtr entity = scene->GetEntity(entity_id_);
            if(entity->HasComponent(type_combo_box_->currentText(), name_line_edit_->text()))
                ok_button_->setEnabled(false);
            else
                ok_button_->setEnabled(true);
        }
    }

    void AddComponentDialog::Initialize()
    {
        if (graphicsProxyWidget())
            graphicsProxyWidget()->setWindowTitle(tr("Add new component ..."));
        else
            setWindowTitle(tr("Add new component ..."));

        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->setContentsMargins(5,5,5,5);
        layout->setSpacing(6);

        component_type_label_ = new QLabel(tr("Component:"));
        component_name_label_ = new QLabel(tr("Name:"));
        component_synch_label_ = new QLabel(tr("Synchronization:"));
        name_line_edit_ = new QLineEdit();
        if(name_line_edit_)
            connect(name_line_edit_, SIGNAL(textChanged(const QString&)), this, SLOT(CheckComponentName(const QString&)));
        type_combo_box_ = new QComboBox();
        synch_combo_box_ = new QComboBox();
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

        ok_button_ = new QPushButton(tr("Ok"));
        if(ok_button_)
            connect(ok_button_, SIGNAL(clicked()), this, SLOT(accept()));
        cancel_button_ = new QPushButton(tr("Cancel"));
        if(cancel_button_)
            connect(cancel_button_, SIGNAL(clicked()), this, SLOT(reject()));
        buttons_layout->addWidget(ok_button_);
        buttons_layout->addWidget(cancel_button_); 
    }
}
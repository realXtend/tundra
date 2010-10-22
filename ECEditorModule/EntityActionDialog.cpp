/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EntityActionDialog.cpp
 *  @brief  
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "EntityActionDialog.h"

#include "MemoryLeakCheck.h"

EntityActionDialog::EntityActionDialog(const QList<Scene::EntityWeakPtr> &entities, const QString &act,
    QWidget *parent, Qt::WindowFlags f) :
    QDialog(parent, f),
    action(act)
{
    this->entities = entities;

    setAttribute(Qt::WA_DeleteOnClose);
    if (graphicsProxyWidget())
        graphicsProxyWidget()->setWindowTitle(tr("Trigger Entity Action"));

    setWindowTitle(tr("Trigger Entity Action"));

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(5,5,5,5);
    layout->setSpacing(6);

    QLabel *actionLabel = new QLabel(tr("Action: ") + action, this);
    QLabel *executionTypeLabel = new QLabel(tr("Execution type:"), this);
    QLabel *parametersLabel = new QLabel(tr("Parameters (use space as separator):"), this);
    parametersLineEdit = new QLineEdit(this);

//    connect(parametersLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(CheckComponentName(const QString&)));
//    connect(typeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(CheckComponentName(const QString&)));

    typeComboBox = new QComboBox(this);
    // Note: If you change the order here, modify also ExecutionType() function logic.
    typeComboBox->addItem(tr("Local"));
    typeComboBox->addItem(tr("Server"));
    typeComboBox->addItem(tr("Peers"));
    typeComboBox->addItem(tr("Local + Server"));
    typeComboBox->addItem(tr("Local + Peers"));
    typeComboBox->addItem(tr("Server + Peers"));
    typeComboBox->addItem(tr("Local + Server + Peers"));

    layout->addWidget(actionLabel);
    layout->addWidget(executionTypeLabel);
    layout->addWidget(typeComboBox);
    layout->addWidget(parametersLabel);
    layout->addWidget(parametersLineEdit);

    QSpacerItem *spacer = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);
    layout->insertSpacerItem(-1, spacer);

    QHBoxLayout *buttonsLayout = new QHBoxLayout(this);
    layout->insertLayout(-1, buttonsLayout);
    QSpacerItem *buttonSpacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    buttonsLayout->addSpacerItem(buttonSpacer);

    QPushButton *okButton = new QPushButton(tr("Ok"));
    QPushButton *cancelButton = new QPushButton(tr("Cancel"));

    connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

    buttonsLayout->addWidget(okButton);
    buttonsLayout->addWidget(cancelButton);
}

EntityActionDialog::~EntityActionDialog()
{
    std::cout << "jhraejgfda" << std::endl;
}

QList<Scene::EntityWeakPtr> EntityActionDialog::Entities() const
{
    return entities;
}

QString EntityActionDialog::Action() const
{
    return action;
}

EntityAction::ExecutionType EntityActionDialog::ExecutionType() const
{
    EntityAction::ExecutionType type;
    switch(typeComboBox->currentIndex())
    {
    case 0:
    default:
        type = EntityAction::Local;
        break;
    case 1:
        type = EntityAction::Server;
        break;
    case 2:
        type = EntityAction::Peers;
        break;
    case 3:
        type = (EntityAction::ExecutionType)(EntityAction::Local | EntityAction::Server);
        break;
    case 4:
        type = (EntityAction::ExecutionType)(EntityAction::Local | EntityAction::Peers);
        break;
    case 5:
        type = (EntityAction::ExecutionType)(EntityAction::Server | EntityAction::Peers);
        break;
    case 6:
        type = (EntityAction::ExecutionType)(EntityAction::Local | EntityAction::Server | EntityAction::Peers);
        break;
    }

    return type;
}

QStringList EntityActionDialog::Parameters() const
{
    return parametersLineEdit->text().split(' ');
}

void EntityActionDialog::hideEvent(QHideEvent *)
{
    close();
}


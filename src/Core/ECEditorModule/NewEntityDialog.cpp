// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "NewEntityDialog.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QCheckBox>
#include <QPushButton>

AddEntityDialog::AddEntityDialog(QWidget *parent, Qt::WindowFlags flags) :
    QDialog(parent, flags)
{
    setModal(true);
    setWindowTitle(tr("Create New Entity"));

    if (graphicsProxyWidget())
        graphicsProxyWidget()->setWindowTitle(windowTitle());

    // Init ui
    QPushButton *buttonCreate = new QPushButton(tr("Create"));
    QPushButton *buttonCancel = new QPushButton(tr("Cancel"));
    buttonCreate->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    buttonCancel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    buttonCreate->setDefault(true);
    buttonCancel->setAutoDefault(false);

    editName_ = new QLineEdit();
    editName_->setFocus(Qt::ActiveWindowFocusReason);
    editName_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    checkBoxSync_ = new QCheckBox(this);
    checkBoxSync_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    checkBoxTemp_ = new QCheckBox(this);
    checkBoxTemp_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QLabel *lName = new QLabel(tr("Name"));
    QLabel *lSync = new QLabel(tr("Local"));
    QLabel *lTemp = new QLabel(tr("Temporary"));
    lTemp->setMinimumWidth(70);

    QGridLayout *grid = new QGridLayout();
    grid->setVerticalSpacing(8);
    grid->addWidget(lName, 0, 0);
    grid->addWidget(editName_, 0, 1, Qt::AlignLeft, 1);
    grid->addWidget(lSync, 1, 0);
    grid->addWidget(checkBoxSync_, 1, 1, Qt::AlignLeft, 1);
    grid->addWidget(lTemp, 2, 0);
    grid->addWidget(checkBoxTemp_, 2, 1, Qt::AlignLeft, 1);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(buttonCreate);
    buttonLayout->addWidget(buttonCancel);

    QVBoxLayout *vertLayout = new QVBoxLayout();
    vertLayout->addLayout(grid);
    vertLayout->addSpacerItem(new QSpacerItem(1,1, QSizePolicy::Fixed, QSizePolicy::Expanding));
    vertLayout->addLayout(buttonLayout);

    setLayout(vertLayout);

    connect(editName_, SIGNAL(returnPressed()), this, SLOT(accept()));
    connect(buttonCreate, SIGNAL(clicked()), this, SLOT(accept()));
    connect(buttonCancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(checkBoxSync_, SIGNAL(toggled(bool)), this, SLOT(CheckTempAndSync()));
    connect(checkBoxTemp_, SIGNAL(toggled(bool)), this, SLOT(CheckTempAndSync()));

    CheckTempAndSync();
}

AddEntityDialog::~AddEntityDialog()
{
}

QString AddEntityDialog::EntityName() const
{
    return editName_->text();
}

bool AddEntityDialog::IsReplicated() const
{
    return !IsLocal();
}

bool AddEntityDialog::IsLocal() const
{
    return checkBoxSync_->isChecked();
}

bool AddEntityDialog::IsTemporary() const
{
    return checkBoxTemp_->isChecked();
}

void AddEntityDialog::CheckTempAndSync()
{
    checkBoxSync_->setText(checkBoxSync_->isChecked() ? "Creating as Local" : "Creating as Replicated");
    checkBoxTemp_->setText(checkBoxTemp_->isChecked() ? "Creating as Temporary" : " ");

    checkBoxSync_->setStyleSheet(checkBoxSync_->isChecked() ? "color: blue;" : "color: black;");
    checkBoxTemp_->setStyleSheet(checkBoxTemp_->isChecked() ? "color: red;" : "color: black;");
}

void AddEntityDialog::showEvent(QShowEvent * /*e*/)
{
    if (editName_)
        editName_->setFocus(Qt::ActiveWindowFocusReason);
}

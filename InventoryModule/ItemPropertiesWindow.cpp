// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file   ItemPropertiesWindow.cpp
 *  @brief  Item properties window. Shows basic information about inventory item and the asset
 *          it's referencing to.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "ItemPropertiesWindow.h"
#include "InventoryModule.h"
#include "InventoryAsset.h"
#include "AbstractInventoryDataModel.h"
#include "ModuleManager.h"
#include "Framework.h"
#include "AssetEvents.h"

#include <QUiLoader>
#include <QFile>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>

#include "MemoryLeakCheck.h"

namespace Inventory
{

ItemPropertiesWindow::ItemPropertiesWindow(InventoryModule *owner, QWidget *parent) :
    QWidget(parent), owner_(owner)
{
    QUiLoader loader;
    QFile file("./data/ui/itemproperties.ui");
    file.open(QFile::ReadOnly);
    QWidget *mainWidget = loader.load(&file, this);
    file.close();

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(mainWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    // Get widgets and connect signals
    lineEditName_ = mainWidget->findChild<QLineEdit *>("lineEditName");
    lineEditDescription_ = mainWidget->findChild<QLineEdit *>("lineEditDescription");
    labelAssetIdData_ = mainWidget->findChild<QLabel *>("labelAssetIdData");
    labelTypeData_ = mainWidget->findChild<QLabel *>("labelTypeData");
    labelFileSizeData_ = mainWidget->findChild<QLabel *>("labelFileSizeData");
    labelCreationTimeData_ = mainWidget->findChild<QLabel *>("labelCreationTimeData");
    labelCreatorData_ = mainWidget->findChild<QLabel *>("labelCreatorData");
    labelOwnerData_ = mainWidget->findChild<QLabel *>("labelOwnerData");
    labelGroupData_ = mainWidget->findChild<QLabel *>("labelGroupData");

    pushButtonSave_ = mainWidget->findChild<QPushButton *>("pushButtonSave");
    pushButtonCancel_ = mainWidget->findChild<QPushButton *>("pushButtonCancel");

    connect(lineEditName_, SIGNAL(editingFinished()), this, SLOT(EditingFinished()));
    connect(lineEditDescription_, SIGNAL(editingFinished()), this, SLOT(EditingFinished()));
//    connect(lineEditName_, SIGNAL(textChanged(const QString &)), this, SLOT(EditingFinished()));
//    connect(lineEditDescription_, SIGNAL(textChanged(const QString &)), this, SLOT(EditingFinished()));

    connect(pushButtonSave_, SIGNAL(clicked()), this, SLOT(Save()));
    connect(pushButtonCancel_, SIGNAL(clicked()), this, SLOT(Cancel()));

    pushButtonSave_->setEnabled(false);
    setWindowTitle(tr("Item Properties"));
}

ItemPropertiesWindow::~ItemPropertiesWindow()
{
}

void ItemPropertiesWindow::SetItem(InventoryAsset *item)
{
    inventoryId_ = item->GetID();
    originalName_ = item->GetName();
    originalDescription_ = item->GetDescription();
    lineEditName_->setText(item->GetName());
    lineEditDescription_->setText(item->GetDescription());
    labelAssetIdData_->setText(item->GetAssetReference());
    labelTypeData_->setText(RexTypes::GetTypeNameFromAssetType(item->GetAssetType()).c_str());
    labelCreationTimeData_->setText(item->GetCreationTimeString());
    creatorId_ = item->GetCreatorId();
    ownerId_ = item->GetOwnerId();
    groupId_ = item->GetGroupId();

    if (!item->IsEditable() || item->IsLibraryItem())
    {
        pushButtonSave_->setEnabled(false);
        lineEditName_->setEnabled(false);
        lineEditDescription_->setEnabled(false);
    }
}

void ItemPropertiesWindow::SetFileSize(size_t file_size)
{
    labelFileSizeData_->setText(QString::number(file_size) + " bytes");
}

void ItemPropertiesWindow::HandleUuidNameReply(QMap<RexUUID, QString> uuid_name_map)
{
    QMapIterator<RexUUID, QString> it(uuid_name_map);
    while(it.hasNext())
    {
        it.next();
        if (it.key() == creatorId_)
            labelCreatorData_->setText(it.value());
        else if (it.key() == ownerId_)
            labelOwnerData_->setText(it.value());
        else if (it.key() == groupId_)
            labelGroupData_->setText(it.value());
    }
}

void ItemPropertiesWindow::Save()
{
    InventoryAsset *asset = static_cast<InventoryAsset *>(owner_->GetInventoryPtr()->GetChildById(inventoryId_));
    assert(asset);
    asset->SetName(lineEditName_->text());
    asset->SetDescription(lineEditDescription_->text());
    emit Closed(inventoryId_, true);
}

void ItemPropertiesWindow::Cancel()
{
    emit Closed(inventoryId_, false);
}

bool ItemPropertiesWindow::EditingFinished()
{
    bool modified = false;

    QString name = lineEditName_->text();
    QString desc = lineEditDescription_->text();

    if (name.isEmpty() || name.isNull())
        lineEditName_->setText(originalName_);

    if (name != originalName_ || desc != originalDescription_)
        modified = true;

    pushButtonSave_->setEnabled(modified);

    return modified;
}

}

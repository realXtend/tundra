// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file   ItemPropertiesWindow.cpp
 *  @brief  Item properties window. Shows basic information about inventory item and the asset
 *          it's referencing to.
 */

#include "StableHeaders.h"
#include "ItemPropertiesWindow.h"
#include "InventoryModule.h"
#include "InventoryAsset.h"
#include "ModuleManager.h"
#include "UiSceneManager.h"
#include "Framework.h"

#include <AssetEvents.h>

#include <UiModule.h>
#include <UiProxyWidget.h>
#include <UiWidgetProperties.h>

#include <QUiLoader>
#include <QFile>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>

namespace Inventory
{

ItemPropertiesWindow::ItemPropertiesWindow(InventoryModule *owner, QWidget *parent) :
    QWidget(parent), owner_(owner)
{
    QUiLoader loader;
    QFile file("./data/ui/itemproperties.ui");
    file.open(QFile::ReadOnly);
    mainWidget_ = loader.load(&file);
    file.close();

    layout_ = new QVBoxLayout;
    layout_->addWidget(mainWidget_);
    layout_->setContentsMargins(0, 0, 0, 0);
    setLayout(layout_);

    // Get widgets and connect signals
    lineEditName_ = mainWidget_->findChild<QLineEdit *>("lineEditName");
    lineEditDescription_ = mainWidget_->findChild<QLineEdit *>("lineEditDescription");

    labelAssetIdData_ = mainWidget_->findChild<QLabel *>("labelAssetIdData");
    labelTypeData_ = mainWidget_->findChild<QLabel *>("labelTypeData");
    labelFileSizeData_ = mainWidget_->findChild<QLabel *>("labelFileSizeData");
    labelCreationTimeData_ = mainWidget_->findChild<QLabel *>("labelCreationTimeData");
    labelCreatorData_ = mainWidget_->findChild<QLabel *>("labelCreatorData");
    labelOwnerData_ = mainWidget_->findChild<QLabel *>("labelOwnerData");
    labelGroupData_ = mainWidget_->findChild<QLabel *>("labelGroupData");

    pushButtonSave_ = mainWidget_->findChild<QPushButton *>("pushButtonSave");
    pushButtonCancel_ = mainWidget_->findChild<QPushButton *>("pushButtonCancel");

    connect(lineEditName_, SIGNAL(editingFinished()), this, SLOT(EditingFinished()));
    connect(lineEditDescription_, SIGNAL(editingFinished()), this, SLOT(EditingFinished()));
    connect(pushButtonSave_, SIGNAL(clicked()), this, SLOT(Save()));
    connect(pushButtonCancel_, SIGNAL(clicked()), this, SLOT(Cancel()));

    pushButtonSave_->setEnabled(false);

    // Add widget to UI via ui services module
    boost::shared_ptr<UiServices::UiModule> ui_module =
        owner_->GetFramework()->GetModuleManager()->GetModule<UiServices::UiModule>(Foundation::Module::MT_UiServices).lock();
    if (!ui_module.get())
        return;

    proxyWidget_ = ui_module->GetSceneManager()->AddWidgetToScene(
        this, UiServices::UiWidgetProperties("Item Properties", UiServices::SceneWidget));

    QObject::connect(proxyWidget_, SIGNAL(Closed()), this, SLOT(Cancel()));

    proxyWidget_->show();
    ui_module->GetSceneManager()->BringProxyToFront(proxyWidget_);
}

ItemPropertiesWindow::~ItemPropertiesWindow()
{
    proxyWidget_->hide();
    mainWidget_->close();
    SAFE_DELETE(layout_);
    SAFE_DELETE(mainWidget_);
}

void ItemPropertiesWindow::SetItem(InventoryAsset *item)
{
    inventoryId_ = item->GetID();
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
        if (it.key() == ownerId_)
            labelOwnerData_->setText(it.value());
        if (it.key() == groupId_)
            labelGroupData_->setText(it.value());
    }
}

void ItemPropertiesWindow::Save()
{
    proxyWidget_->hide();

    boost::shared_ptr<UiServices::UiModule> ui_module =
        owner_->GetFramework()->GetModuleManager()->GetModule<UiServices::UiModule>(Foundation::Module::MT_UiServices).lock();
    if (!ui_module.get())
        return;

    ui_module->GetSceneManager()->RemoveProxyWidgetFromScene(proxyWidget_);
    owner_->CloseItemPropertiesWindow(inventoryId_, true);
}

void ItemPropertiesWindow::Cancel()
{
    proxyWidget_->hide();

    boost::shared_ptr<UiServices::UiModule> ui_module =
        owner_->GetFramework()->GetModuleManager()->GetModule<UiServices::UiModule>(Foundation::Module::MT_UiServices).lock();
    if (!ui_module.get())
        return;

    ui_module->GetSceneManager()->RemoveProxyWidgetFromScene(proxyWidget_);
    owner_->CloseItemPropertiesWindow(inventoryId_, false);
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

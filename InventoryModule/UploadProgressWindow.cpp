// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file   UploadProgressWindow
 *  @brief  A window showing progress bar for current uploads.
 */

#include "StableHeaders.h"
#include "UploadProgressWindow.h"
#include "InventoryModule.h"
#include "ModuleManager.h"
#include "Inworld/InworldSceneController.h"
#include "Framework.h"

#include <UiModule.h>
#include "Inworld/View/UiProxyWidget.h"
#include "Inworld/View/UiWidgetProperties.h"

#include <QUiLoader>
#include <QFile>
#include <QVBoxLayout>
#include <QString>
#include <QProgressBar>
#include <QLabel>

namespace Inventory
{

UploadProgressWindow::UploadProgressWindow(InventoryModule *owner, QWidget *parent) :
    QWidget(parent), owner_(owner), mainWidget_(0), layout_(0), uploadCount_(0)
{
    QUiLoader loader;
    QFile file("./data/ui/uploadprogress.ui");
    file.open(QFile::ReadOnly);
    mainWidget_ = loader.load(&file, this);
    file.close();

    layout_ = new QVBoxLayout;
    layout_->addWidget(mainWidget_);
    setLayout(layout_);

    progressBar_ = mainWidget_->findChild<QProgressBar *>("progressBar");
    labelFileNumber_ = mainWidget_->findChild<QLabel *>("labelFileNumber");

    // Add widget to UI via ui services module
    boost::shared_ptr<UiServices::UiModule> ui_module =
        owner_->GetFramework()->GetModuleManager()->GetModule<UiServices::UiModule>(Foundation::Module::MT_UiServices).lock();
    if (!ui_module.get())
        return;

    proxyWidget_ = ui_module->GetInworldSceneController()->AddWidgetToScene(
        this, UiServices::UiWidgetProperties("Upload Progress Window", UiServices::SceneWidget));
}

UploadProgressWindow::~UploadProgressWindow()
{
    proxyWidget_->hide();
    mainWidget_->close();
    SAFE_DELETE(layout_);
    SAFE_DELETE(mainWidget_);
}

void UploadProgressWindow::OpenUploadProgress(size_t file_count)
{
    boost::shared_ptr<UiServices::UiModule> ui_module =
        owner_->GetFramework()->GetModuleManager()->GetModule<UiServices::UiModule>(Foundation::Module::MT_UiServices).lock();
    if (!ui_module.get())
        return;

    progressBar_->setRange(0, file_count);
    progressBar_->setValue(uploadCount_);
    proxyWidget_->show();
    ui_module->GetInworldSceneController()->BringProxyToFront(proxyWidget_);
}

void UploadProgressWindow::UploadStarted(const QString &filename)
{
    ++uploadCount_;
    int max_value = progressBar_->maximum();
    if (uploadCount_ <= max_value)
    {
        progressBar_->setValue(uploadCount_);
        labelFileNumber_->setText(QString("%1 (%2/%3)").arg(filename).arg(uploadCount_).arg(max_value));
    }
}

void UploadProgressWindow::CloseUploadProgress()
{
    progressBar_->reset();
    uploadCount_ = 0;
    proxyWidget_->hide();
}

}

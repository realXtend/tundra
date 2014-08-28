// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "SaveSceneDialog.h"
#include "Framework.h"
#include "ConfigAPI.h"
#include "UiAPI.h"
#include "UiMainWindow.h"

#include "FileUtils.h"
#include "SupportedFileTypes.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QCheckBox>
#include <QPushButton>

#include "MemoryLeakCheck.h"

SaveSceneDialog::SaveSceneDialog(Framework *fw, QWidget *parent, Qt::WindowFlags flags) :
    QDialog(parent, flags),
    framework_(fw)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setModal(true);
    setWindowTitle(tr("Save Scene"));
    setMinimumWidth(550);
    setMinimumHeight(150);

    if (graphicsProxyWidget())
        graphicsProxyWidget()->setWindowTitle(windowTitle());

    // Init ui
    QPushButton *buttonSave = new QPushButton(tr("Save"));
    QPushButton *buttonCancel = new QPushButton(tr("Cancel"));
    buttonSave->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    buttonCancel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    buttonSave->setDefault(true);
    buttonCancel->setAutoDefault(false);

    QHBoxLayout *fileLayout = new QHBoxLayout();
    editFile_ = new QLineEdit();
    editFile_->setFocus(Qt::ActiveWindowFocusReason);
    editFile_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    
    QPushButton *buttonPickFile = new QPushButton(tr("..."));
    fileLayout->addWidget(editFile_);
    fileLayout->addWidget(buttonPickFile);

    checkBoxSaveLocal_ = new QCheckBox(this);
    checkBoxSaveLocal_->setMinimumHeight(17);
    checkBoxSaveLocal_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    checkBoxSaveTemp_ = new QCheckBox(this);
    checkBoxSaveTemp_->setMinimumHeight(17);
    checkBoxSaveTemp_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QLabel *lPath = new QLabel(tr("File"));
    QLabel *lSync = new QLabel(tr("Local Entities"));
    QLabel *lTemp = new QLabel(tr("Temporary Entities"));
    lTemp->setMinimumWidth(100);

    QGridLayout *grid = new QGridLayout();
    grid->setVerticalSpacing(8);
    grid->addWidget(lPath, 0, 0);
    grid->addLayout(fileLayout, 0, 1, Qt::AlignLeft, 1);
    grid->addWidget(lSync, 1, 0);
    grid->addWidget(checkBoxSaveLocal_, 1, 1, Qt::AlignLeft, 1);
    grid->addWidget(lTemp, 2, 0);
    grid->addWidget(checkBoxSaveTemp_, 2, 1, Qt::AlignLeft, 1);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(buttonSave);
    buttonLayout->addWidget(buttonCancel);

    QVBoxLayout *vertLayout = new QVBoxLayout();
    vertLayout->addLayout(grid);
    vertLayout->addSpacerItem(new QSpacerItem(1,1, QSizePolicy::Fixed, QSizePolicy::Expanding));
    vertLayout->addLayout(buttonLayout);

    setLayout(vertLayout);
    
    // Fill data from config
    editFile_->setText(framework_->Config()->Read("uimemory", "save scene dialog", "file", "").toString());
    checkBoxSaveLocal_->setChecked(framework_->Config()->Read("uimemory", "save scene dialog", "save local", false).toBool());
    checkBoxSaveTemp_->setChecked(framework_->Config()->Read("uimemory", "save scene dialog", "save temporary", false).toBool());

    CheckTempAndSync();

    connect(editFile_, SIGNAL(returnPressed()), this, SLOT(OnAccepted()));
    connect(buttonPickFile, SIGNAL(clicked()), this, SLOT(PickFile()));
    connect(buttonSave, SIGNAL(clicked()), this, SLOT(OnAccepted()));
    connect(buttonCancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(checkBoxSaveLocal_, SIGNAL(toggled(bool)), this, SLOT(CheckTempAndSync()));
    connect(checkBoxSaveTemp_, SIGNAL(toggled(bool)), this, SLOT(CheckTempAndSync()));
}

SaveSceneDialog::~SaveSceneDialog()
{
}

void SaveSceneDialog::showEvent(QShowEvent *e)
{
    if (editFile_)
        editFile_->setFocus(Qt::ActiveWindowFocusReason);
    CenterToMainWindow();
    QDialog::showEvent(e);
}

void SaveSceneDialog::CenterToMainWindow()
{
    if (framework_->Ui()->MainWindow())
    {
        QRect mainRect = framework_->Ui()->MainWindow()->rect();
        QPoint mainCenter = framework_->Ui()->MainWindow()->pos() + mainRect.center();
        move(mainCenter.x()-width()/2, (mainCenter.y()-height()/2) >= 0 ? mainCenter.y()-height()/2 : 0);
    }
}

QString SaveSceneDialog::File() const
{
    return editFile_->text().trimmed();
}

bool SaveSceneDialog::SaveLocalEntities() const
{
    return checkBoxSaveLocal_->isChecked();
}

bool SaveSceneDialog::SaveTemporaryEntities() const
{
    return checkBoxSaveTemp_->isChecked();
}

void SaveSceneDialog::PickFile()
{
    QString dir;
    if (!File().isEmpty())
    {
        QFileInfo fi(File());
        dir = fi.isDir() ? fi.absolutePath() : fi.absoluteDir().absolutePath();
    }

    SaveFileDialogNonModal(cTundraXmlFileFilter + ";;" + cTundraBinaryFileFilter,
        tr("Save Scene As"), dir, this, this, SLOT(OnFilePicked(int)));
}

void SaveSceneDialog::SetFile(QString file, bool xml)
{
    // Complete extension or detect binary/xml
    if (file.lastIndexOf('.') == -1)
    {
        if (xml)
            file.append(cTundraXmlFileExtension);
        else
            file.append(cTundraBinFileExtension);
    }
    file = file.trimmed();
    editFile_->setText(file);
    framework_->Config()->Write("uimemory", "save scene dialog", "file", file);
}

void SaveSceneDialog::OnFilePicked(int result)
{
    QFileDialog *dialog = qobject_cast<QFileDialog *>(sender());
    if (!dialog || result != QDialog::Accepted)
        return;
    QStringList files = dialog->selectedFiles();
    if (files.size() == 1)
        SetFile(files.first(), dialog->selectedNameFilter() == cTundraXmlFileFilter);
}

void SaveSceneDialog::OnAccepted()
{
    if (File().isEmpty())
    {
        QMessageBox::information(this, "Invalid File", "No target file selected", QMessageBox::Ok, QMessageBox::Ok);
        return;
    }
    framework_->Config()->Write("uimemory", "save scene dialog", "file", File());
    emit Selected(File(), true, SaveTemporaryEntities(), SaveLocalEntities());

    // Closes dialog
    accept();
}

void SaveSceneDialog::CheckTempAndSync()
{
    checkBoxSaveLocal_->setText(checkBoxSaveLocal_->isChecked() ? "Saving Local" : "");
    checkBoxSaveTemp_->setText(checkBoxSaveTemp_->isChecked() ? "Saving Temporary" : "");

    checkBoxSaveLocal_->setStyleSheet(checkBoxSaveLocal_->isChecked() ? "color: blue;" : "color: black;");
    checkBoxSaveTemp_->setStyleSheet(checkBoxSaveTemp_->isChecked() ? "color: red;" : "color: black;");

    framework_->Config()->Write("uimemory", "save scene dialog", "save local", checkBoxSaveLocal_->isChecked());
    framework_->Config()->Write("uimemory", "save scene dialog", "save temporary", checkBoxSaveTemp_->isChecked());
}

/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   CloneAssetDialog.cpp
 *  @brief  Dialog for cloning asset.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "CloneAssetDialog.h"

#include "IAsset.h"
#include "AssetAPI.h"

#include "MemoryLeakCheck.h"

CloneAssetDialog::CloneAssetDialog(const AssetPtr &asset, AssetAPI *assetApi, QWidget *parent, Qt::WindowFlags f) :
    QDialog(parent, f)
{
    this->asset = asset;
    this->assetApi = assetApi;

    setWindowTitle(tr("Clone Asset: ") + asset->Name());
    setAttribute(Qt::WA_DeleteOnClose);
    if (graphicsProxyWidget())
        graphicsProxyWidget()->setWindowTitle(windowTitle());

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(5,5,5,5);
    //layout->setSpacing(6);

    QLabel *newNameLabel = new QLabel(tr("New name:"), this);
    nameLineEdit = new QLineEdit;
    errorLabel = new QLabel(this);

    QHBoxLayout *buttonLayout = new QHBoxLayout(this);
    QSpacerItem *spacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    okButton = new QPushButton(tr("OK"), this);
    buttonLayout->addSpacerItem(spacer);
    buttonLayout->addWidget(okButton);

    layout->addWidget(newNameLabel);
    layout->addWidget(nameLineEdit);
    layout->addWidget(errorLabel);
    layout->addLayout(buttonLayout);

    connect(okButton, SIGNAL(clicked()), SLOT(accept()));
    connect(nameLineEdit, SIGNAL(textChanged(const QString &)), SLOT(ValidateNewName(const QString &)));
    //connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

QString CloneAssetDialog::NewName() const
{
    return nameLineEdit->text();
}

void CloneAssetDialog::hideEvent(QHideEvent * /*e*/)
{
    deleteLater();
}

void CloneAssetDialog::ValidateNewName(const QString &newName)
{
    if (newName.trimmed().isEmpty())
    {
        okButton->setEnabled(false);
        errorLabel->setText(tr("Cannot use an empty name!"));
    }
    else if (assetApi->GetAsset(newName))
    {
        okButton->setEnabled(false);
        errorLabel->setText(tr("Name already taken!"));
    }
    else
    {
        okButton->setEnabled(true);
        errorLabel->clear();
    }
}

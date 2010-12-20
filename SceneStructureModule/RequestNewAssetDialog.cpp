/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   RequestNewAssetDialog.cpp
 *  @brief  Dialog for requesting new asset from arbitrary source.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "RequestNewAssetDialog.h"

#include "AssetAPI.h"
#include "IAssetTypeFactory.h"

#include "MemoryLeakCheck.h"

RequestNewAssetDialog::RequestNewAssetDialog(AssetAPI *assetApi, QWidget *parent, Qt::WindowFlags f) :
    QDialog(parent, f)
{
    setWindowTitle(tr("Request New Asset"));
    setAttribute(Qt::WA_DeleteOnClose);
    if (graphicsProxyWidget())
        graphicsProxyWidget()->setWindowTitle(windowTitle());

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(5,5,5,5);
    //layout->setSpacing(6);

    QLabel *sourceLabel = new QLabel(tr("Source"), this);
    sourceLineEdit = new QLineEdit;
    QLabel *typeLabel = new QLabel(tr("Type"), this);

    typeComboBox = new QComboBox;
    typeComboBox->setEditable(true);
    typeComboBox->addItem("");
    foreach(AssetTypeFactoryPtr factory, assetApi->GetAssetTypeFactories())
        typeComboBox->addItem(factory->Type());

    QHBoxLayout *buttonLayout = new QHBoxLayout(this);
    QSpacerItem *spacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    QPushButton* okButton = new QPushButton(tr("OK"), this);
    buttonLayout->addSpacerItem(spacer);
    buttonLayout->addWidget(okButton);

    layout->addWidget(sourceLabel);
    layout->addWidget(sourceLineEdit);
    layout->addWidget(typeLabel);
    layout->addWidget(typeComboBox);
    layout->addLayout(buttonLayout);

    connect(okButton, SIGNAL(clicked()), SLOT(accept()));
    //connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

QString RequestNewAssetDialog::Source() const
{
    return sourceLineEdit->text();
}

QString RequestNewAssetDialog::Type() const
{
    return typeComboBox->currentText();
}

void RequestNewAssetDialog::hideEvent(QHideEvent * /*e*/)
{
    deleteLater();
}

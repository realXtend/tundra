#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "MultiEditWidget.h"
#include "ECEditorModule.h"

#include <QInputDialog>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QPushButton>
#include <QLayout>
#include <QInputDialog>

#include <QUiLoader>
#include <QFile>

#include "MemoryLeakCheck.h"

namespace ECEditor
{
    /*MultiEditDialog::MultiEditDialog(QWidget *parent, Qt::WindowFlags flag): QDialog(parent, flag)
    {
        verticalLayout_ = new QVBoxLayout(this);
        list_ = new QListWidget();
        lineEdit_ = new QLineEdit();
        verticalLayout_->addWidget(list_);
        verticalLayout_->addWidget(lineEdit_);
        if(list_)
            QObject::connect(list_, SIGNAL(itemSelectionChanged()), this, SLOT(updateLineEditText()));

        QHBoxLayout *hLayout = new QHBoxLayout();
        //verticalLayout_->addItem(hLayout);

        okButton_ = new QPushButton();
        if(okButton_)
        {
            okButton_->setText("Ok");
            QObject::connect(okButton_, SIGNAL(clicked()), this, SLOT(accept()));
            hLayout->addWidget(okButton_);
        }

        cancelButton_ = new QPushButton();
        if(cancelButton_)
        {
            cancelButton_->setText("Cancel");
            QObject::connect(cancelButton_, SIGNAL(clicked()), this, SLOT(reject()));
            hLayout->addWidget(cancelButton_);
        }
    }

    MultiEditDialog::~MultiEditDialog()
    {

    }

    void MultiEditDialog::AddAttributeValues(QVector<QString> attributeValues)
    {
        if(!list_)
            return;

        for(uint i = 0; i < attributeValues.size(); i++)
            list_->addItem(attributeValues[i]);
    }

    QString MultiEditDialog::GetAttributeValue() const
    {
        if(lineEdit_)
            return lineEdit_->text();
        return "";
    }

    void MultiEditDialog::UpdateLineEditText()
    {
        if(list_ && lineEdit_)
        {
            QString selectedText = list_->currentItem()->text();
            lineEdit_->setText(selectedText);
        }
    }*/

    MultiEditWidget::MultiEditWidget(QWidget *parent):
        QPushButton(parent),
        dialog_(0)
    {
        setText("Multiedit");
        selectedValue_ = QString("");
        QObject::connect(this, SIGNAL(clicked()), this, SLOT(CreateMultiSelectDialog()));
    }

    MultiEditWidget::~MultiEditWidget()
    {

    }

    void MultiEditWidget::SetAttributeValues(QStringList attributeValues)
    {
        attributeValues_ = attributeValues;
    }

    void MultiEditWidget::CreateMultiSelectDialog()
    {
        if(dialog_)
            return;

        QStringList items;
        for(uint i = 0; i < attributeValues_.size(); i++)
            items << attributeValues_[i];

        dialog_ = new QInputDialog(this);
        if(dialog_)
        {
            dialog_->setInputMode(QInputDialog::TextInput);
            dialog_->setComboBoxEditable(true);
            dialog_->setComboBoxItems(items);
            dialog_->setOption(QInputDialog::UseListViewForComboBoxItems);
            QObject::connect(dialog_, SIGNAL(textValueSelected(const QString&)), this, SLOT(GetFinalResultFromDialog(const QString&)));
            dialog_->exec();
        }
    }

    void MultiEditWidget::GetFinalResultFromDialog(const QString &select)
    {
        selectedValue_ = select;
        emit ValueSelected(select);
    }
}
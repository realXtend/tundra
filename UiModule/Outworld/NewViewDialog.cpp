//$ HEADER_NEW_FILE $
#include "StableHeaders.h"

#include "NewViewDialog.h"

#include <QLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QList>

namespace UiServices
{
	NewViewDialog::NewViewDialog(QStringList views, QWidget *parent, Qt::WindowFlags f):
		QDialog(parent, f),
		views_(views),
		name_line_edit_(0),
		save_button_(0),
		cancel_button_(0)
	{
		setAttribute(Qt::WA_DeleteOnClose);
		setModal(true);
		setWindowTitle(tr("New View"));

		//Create layout
		QVBoxLayout *layout = new QVBoxLayout(this);
		this->setLayout(layout);
		
		//Create Objects
		QLabel* title_label= new QLabel(tr("Create View"),this);
		name_line_edit_ = new QLineEdit(this);

		save_button_ = new QPushButton(tr("Save"), this);
		cancel_button_ = new QPushButton(tr("Cancel"), this);

		//Configure objects
		title_label->setAlignment(Qt::AlignCenter);

		layout->addWidget(title_label,0,Qt::AlignCenter);

		layout->addWidget(name_line_edit_,1,Qt::AlignCenter);

		QHBoxLayout *hlayout = new QHBoxLayout(this);
		hlayout->addWidget(save_button_,0,Qt::AlignLeft);
		hlayout->addWidget(cancel_button_,1,Qt::AlignRight);
		layout->addLayout(hlayout,2);

		connect(save_button_, SIGNAL(clicked()), this, SLOT(OnSaveButtonClicked()));
		connect(cancel_button_, SIGNAL(clicked()), this, SLOT(close()));

	}

	NewViewDialog::~NewViewDialog()
	{
	}

	void NewViewDialog::OnSaveButtonClicked()
	{
		if(views_.contains(name_line_edit_->text())){
			QMessageBox* msgInfo=new QMessageBox();
			msgInfo->setText("The view already exist");
			msgInfo->setIcon(QMessageBox::Warning);
			msgInfo->exec();
		}else if(name_line_edit_->text()==""){
			QMessageBox* msgInfo=new QMessageBox();
			msgInfo->setText("The name can't be empty");
			msgInfo->setIcon(QMessageBox::Warning);
			msgInfo->exec();
		}else{
			emit Save(name_line_edit_->text());
			QMessageBox* msgInfo=new QMessageBox();
			msgInfo->setText("The view has been saved");
			msgInfo->setIcon(QMessageBox::Information);
			msgInfo->exec();
			close();
		}
	}
}
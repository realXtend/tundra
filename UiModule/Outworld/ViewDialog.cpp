//$ HEADER_NEW_FILE $
#include "StableHeaders.h"

#include "ViewDialog.h"

#include <QLayout>
#include <QLineEdit>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QList>
#include <QMessageBox>

namespace UiServices
{
	ViewDialog::ViewDialog(QStringList views, QWidget *parent, Qt::WindowFlags f):
		QDialog(parent, f),
		views_(views),
		name_line_edit_(0),
		view_combo_box_(0),
		save_button_(0),
		cancel_button_(0)
	{
		setWindowTitle(tr("Configure Views"));
		setModal(true);
		this->adjustSize();
		views_.removeOne("Building");
		views_.removeOne("Previous");

		//Create layout
		QGridLayout *layout = new QGridLayout(this);
		this->setLayout(layout);
		
		//Create Objects
		QLabel* title_label= new QLabel(tr("Configuration Views"),this);
		QLabel* name_label= new QLabel(tr("New name"),this);
		name_line_edit_ = new QLineEdit(this);
		view_combo_box_ = new QComboBox(this);
		rename_button_ = new QPushButton(tr("Rename"), this);
		save_button_ = new QPushButton(tr("Save"), this);
		cancel_button_ = new QPushButton(tr("Close"), this);
		delete_button_ = new QPushButton(tr("Delete"), this);

		//Configure objects
		title_label->setAlignment(Qt::AlignCenter);
		title_label->setGeometry(QRect(30, 50, 301, 61));
		QFont font;
		font.setPointSize(18);
		font.setWeight(50);
		title_label->setFont(font);
		view_combo_box_->addItems(views_);
		view_combo_box_->setFixedSize(150,30);

		layout->addWidget(title_label,0,0,1,3,Qt::AlignCenter);

		layout->addWidget(view_combo_box_,1,1,1,2,Qt::AlignLeft);
		
		layout->addWidget(name_label,2,0,Qt::AlignRight);
		layout->addWidget(name_line_edit_,2,1,Qt::AlignLeft);
		layout->addWidget(rename_button_,2,2,Qt::AlignLeft);

		layout->addWidget(save_button_,3,0,Qt::AlignRight);
		layout->addWidget(delete_button_,3,1,Qt::AlignCenter);
		layout->addWidget(cancel_button_,3,2,Qt::AlignLeft);

		connect(rename_button_, SIGNAL(clicked()), this, SLOT(OnRenameButtonClicked()));
		connect(save_button_, SIGNAL(clicked()), this, SLOT(OnSaveButtonClicked()));
		connect(delete_button_, SIGNAL(clicked()), this, SLOT(OnDeleteButtonClicked()));
		connect(cancel_button_, SIGNAL(clicked()), this, SLOT(hide()));

	}

	ViewDialog::~ViewDialog()
	{
	}

	void ViewDialog::OnRenameButtonClicked()
	{
		if(view_combo_box_->count()==0){
			QMessageBox* msgInfo=new QMessageBox();
			msgInfo->setText("There aren't any view to rename");
			msgInfo->setIcon(QMessageBox::Information);
			msgInfo->exec();
		}else{
			if(name_line_edit_->text()==""){
				QMessageBox* msgInfo=new QMessageBox();
				msgInfo->setText("The name can't be empty");
				msgInfo->setIcon(QMessageBox::Information);
				msgInfo->exec();
			}else if(name_line_edit_->text()=="Previous" && name_line_edit_->text()=="Building"){
				QMessageBox* msgInfo=new QMessageBox();
				msgInfo->setText("This name is not available");
				msgInfo->setIcon(QMessageBox::Information);
				msgInfo->exec();
			}else if (views_.contains(name_line_edit_->text())){
				QMessageBox* msgInfo=new QMessageBox();
				msgInfo->setText("The view already exist");
				msgInfo->setIcon(QMessageBox::Information);
				msgInfo->exec();
			}else
				emit Rename(view_combo_box_->currentText(),name_line_edit_->text());
		}
	}

	void ViewDialog::OnSaveButtonClicked()
	{
		if(view_combo_box_->count()==0){
			QMessageBox* msgInfo=new QMessageBox();
			msgInfo->setText("There aren't any view to save");
			msgInfo->setIcon(QMessageBox::Information);
			msgInfo->exec();
		}else{
			QMessageBox* msgBox=new QMessageBox();
			msgBox->setText("The view has been modified");
			msgBox->setInformativeText("Do you want to save your changes in  "+view_combo_box_->currentText());
			msgBox->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
			msgBox->setDefaultButton(QMessageBox::Yes);
			msgBox->setIcon(QMessageBox::Question);
			int option = msgBox->exec();
			QMessageBox* msgInfo=new QMessageBox();
			switch (option) {
				case QMessageBox::Yes:
					emit Save(view_combo_box_->currentText());
					msgInfo->setText("The view "+view_combo_box_->currentText()+" has been saved");
					msgInfo->setIcon(QMessageBox::Information);
					msgInfo->exec();
					break;
				case QMessageBox::No:
					// Don't Save was clicked
					break;
				default:
					// should never be reached
					break;
			}
		}
	}

	void ViewDialog::OnDeleteButtonClicked()
	{
		if(view_combo_box_->count()==0){
			QMessageBox* msgInfo=new QMessageBox();
			msgInfo->setText("There aren't any view to save");
			msgInfo->setIcon(QMessageBox::Information);
			msgInfo->exec();
		}else{
			QMessageBox* msgBox=new QMessageBox();
			msgBox->setText("The view will be deleted.");
			msgBox->setInformativeText("Do you want to delete the view "+view_combo_box_->currentText());
			msgBox->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
			msgBox->setDefaultButton(QMessageBox::No);
			msgBox->setIcon(QMessageBox::Question);
			int option = msgBox->exec();
			QMessageBox* msgInfo=new QMessageBox();
			switch (option) {
				case QMessageBox::Yes:
					emit Delete(view_combo_box_->currentText());
					msgInfo->setText("The view has been deleted");
					msgInfo->setIcon(QMessageBox::Information);
					msgInfo->exec();
					break;
				case QMessageBox::No:
					// Don't Save was clicked
					break;
				default:
					// should never be reached
					break;
			}
		}
	}

	void ViewDialog::UpdateViews(QStringList views)
	{
		views_=views;
		view_combo_box_->clear();
		views_.removeOne("Building");
		views_.removeOne("Previous");
		view_combo_box_->addItems(views_);
	}


}
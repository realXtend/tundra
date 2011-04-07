//$ HEADER_NEW_FILE $
#include "StableHeaders.h"
#include "ViewManager.h"
#include "NewViewDialog.h"

#include <QSettings>
#include <QMap>
#include <QList>
#include <QListIterator>
#include <QMessageBox>
#include <QVariantList>

namespace UiServices
{
	ViewManager::ViewManager(UiModule *owner):
		owner_(owner),
		uiService_(0)
	{
		uiService_ = owner_->GetFramework()->GetService<UiServiceInterface>();

		qWin_ = dynamic_cast<QMainWindow*>(owner_->GetFramework()->Ui()->MainWindow());
		QSettings settings(QSettings::IniFormat, QSettings::UserScope, APPLICATION_NAME, "configuration/ConfigurationViews");
		
		if(!settings.childGroups().contains("Building")){
			QSettings build("./data/defaultviews.ini",QSettings::IniFormat);
			settings.beginGroup("Building");
			build.beginGroup("Building");
			QStringList keys = build.childKeys();
			QListIterator<QString> i(keys);
			while(i.hasNext()){
				QString key = i.next();
				settings.setValue(key,build.value(key));
			}	
			build.endGroup();
			settings.endGroup();
		}

		QStringList views;
		views= settings.childGroups();

		//Create the menu
		QMenuBar* menuBar = qWin_->menuBar();
		configWindow_ = new ViewDialog(views);
		menu_ = new QMenu("Window",menuBar);
		menuBar->addMenu(menu_);

		//Insert New Action
		QAction* newAction = new QAction(tr("New"), menu_);
		connect(newAction, SIGNAL(triggered()), SLOT(NewViewWindow()));
		menu_->addAction(newAction);

		//Insert Configure Action
		QAction* open = new QAction(tr("Configure"), menu_);
		connect(open, SIGNAL(triggered()),configWindow_, SLOT(show()));
		menu_->addAction(open);
		menu_->addSeparator();

		//Create the Action Group and insert all views
		actionGroup_=new QActionGroup(menu_);
		connect(actionGroup_, SIGNAL(triggered(QAction*)), SLOT(ActionChanged(QAction*)));

		QAction* hide = new QAction(tr("Hide"), menu_);
		connect(hide, SIGNAL(triggered()), SLOT(HideView()));
		actionGroup_->addAction(hide);
		menu_->addAction(hide);

		QListIterator<QString> i(views);
		while(i.hasNext()){
			QString name=i.next();
			actionGroup_->addAction(new QAction(name,menu_));
		}

		QListIterator<QAction*> q(actionGroup_->actions());
		while(q.hasNext()){
			QAction* a=q.next();
			a->setCheckable(true);
			menu_->addAction(a);
		}

		connect(configWindow_,SIGNAL(Save(const QString &)),this,SLOT(SaveView(const QString &)));		
		connect(configWindow_,SIGNAL(Delete(const QString &)),this,SLOT(DeleteView(const QString &)));		
		connect(configWindow_,SIGNAL(Rename(const QString &,const QString &)),this,SLOT(RenameView(const QString &,const QString &)));

	}

	ViewManager::~ViewManager()
	{
	}

	void ViewManager::NewViewWindow()
	{
		QSettings settings(QSettings::IniFormat, QSettings::UserScope, APPLICATION_NAME, "configuration/ConfigurationViews");
		NewViewDialog* window=new NewViewDialog(settings.childGroups());
		connect(window,SIGNAL(Save(const QString &)),this,SLOT(SaveView(const QString &)));
		window->show();
	}

	void ViewManager::HideView()
	{
		QList<QString> widgets = uiService_->GetAllWidgetsNames();

		QListIterator<QString> i(widgets);
		while(i.hasNext())
			uiService_->HideWidget(i.next());
	}

	void ViewManager::ShowView(const QString &name)
	{
		//Restore values
		QSettings settings(QSettings::IniFormat, QSettings::UserScope, APPLICATION_NAME, "configuration/ConfigurationViews");
		if(settings.childGroups().contains(name)){
			settings.beginGroup(name);

			QList<QString> currentWidgets = uiService_->GetAllWidgetsNames();	
			QStringList widgets = settings.childGroups();
			QListIterator<QString> i(widgets);

			HideView();

			while(i.hasNext()){
				QString widgetName = i.next();
				if(!currentWidgets.contains(widgetName)){
					QString module="";
					QVariantList properties;
					settings.beginGroup(widgetName);
					QStringList keys = settings.childKeys();
					QListIterator<QString> j(keys);
					while(j.hasNext()){
						QString prop = j.next(); //Only has one element
						if(prop=="DP_ModuleName")
							module=settings.value(prop).toString();
						else
							properties.append(settings.value(prop));
					}
					settings.endGroup();
					emit uiService_->SendToCreateDynamicWidget(widgetName,module,properties);
				}
			}
			
			qWin_->restoreState(settings.value("win_state", QByteArray()).toByteArray());		
			settings.endGroup();

		}else{
			if(name!="Hide"){
				QMessageBox* msgInfo=new QMessageBox();
				msgInfo->setText("Sorry, this view is not available");
				msgInfo->setIcon(QMessageBox::Information);
				msgInfo->exec();
			}
		}
	}

	void ViewManager::SaveView(const QString &name)
	{
		////Save settings with QtSettings..
		QSettings settings(QSettings::IniFormat, QSettings::UserScope, APPLICATION_NAME, "configuration/ConfigurationViews");

		if(!settings.childGroups().contains(name)){
			QAction* a = new QAction(name,menu_);
			a->setCheckable(true);
			actionGroup_->addAction(a);
			menu_->addAction(a);
			a->setChecked(true);
		}
		
		settings.beginGroup(name);

		//Delete the old widgets
		QStringList keys = settings.allKeys();
		QListIterator<QString> key(keys);
		while(key.hasNext())
			settings.remove(key.next());
			
		settings.setValue("win_state", qWin_->saveState());

		QList<QString> widgets = uiService_->GetAllWidgetsNames();
		QListIterator<QString> i(widgets);

		while(i.hasNext()){
			QString nameWidget = i.next();
			QWidget* widget = uiService_->GetWidget(nameWidget);
			if(dynamic_cast<QDockWidget*>(widget))
				widget = dynamic_cast<QDockWidget*>(widget)->widget();
			if(widget->property("dynamic").isValid()){
				settings.beginGroup(nameWidget);
				QList<QByteArray> properties = widget->dynamicPropertyNames();
				QListIterator<QByteArray> p(properties);
				while(p.hasNext()){
					QString s = p.next();
					if(s.startsWith("DP_"))
						 settings.setValue(s, widget->property(s.toAscii()));
				}
				settings.endGroup();
			}
		}
		settings.endGroup();
		
		configWindow_->UpdateViews(settings.childGroups());
	}

	void ViewManager::DeleteView(const QString &name)
	{
		QSettings settings(QSettings::IniFormat, QSettings::UserScope, APPLICATION_NAME, "configuration/ConfigurationViews");
		
		QStringList groups = settings.childGroups();
		if(groups.contains(name)){
			QListIterator<QAction*> q(actionGroup_->actions());
			while(q.hasNext()){
				QAction* a=q.next();
				if(a->text()==name)
					menu_->removeAction(a);
			}
			settings.remove(name);
			configWindow_->UpdateViews(settings.childGroups());
		}
	}

	void ViewManager::RenameView(const QString &oldName,const QString &newName)
	{
		QSettings settings(QSettings::IniFormat, QSettings::UserScope, APPLICATION_NAME, "configuration/ConfigurationViews");
		QSettings newSettings(QSettings::IniFormat, QSettings::UserScope, APPLICATION_NAME, "configuration/ConfigurationViews");
		if(settings.childGroups().contains(oldName) && !settings.childGroups().contains(newName)){
			settings.beginGroup(oldName);
			newSettings.beginGroup(newName);
			QStringList keys = settings.childKeys();
			QListIterator<QString> i(keys);
			while(i.hasNext()){
				QString key = i.next();
				newSettings.setValue(key,settings.value(key));
			}	
			QListIterator<QAction*> q(actionGroup_->actions());
			while(q.hasNext()){
				QAction* a=q.next();
				if(a->text()==oldName)
					a->setText(newName);
			}
			DeleteView(oldName);
		}
	}

	void ViewManager::ActionChanged(QAction* action)
	{
		ShowView(actionGroup_->checkedAction()->text());
	}
}
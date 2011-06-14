//$ HEADER_NEW_FILE $
#include "StableHeaders.h"
#include "ViewManager.h"
#include "NewViewDialog.h"
#include "UiMainWindow.h"
#include "UiAPI.h"
#include "IUiWidgetFactory.h"

#include <QSettings>
#include <QMap>
#include <QList>
#include <QSet>
#include <QListIterator>
#include <QMessageBox>
#include <QVariantList>
#include <QMainWindow>
#include <QDockWidget>

namespace UiServices
{
	ViewManager::ViewManager(UiModule *owner):
		owner_(owner),
		uiService_(0)
	{
		//uiService_ = owner_->GetFramework()->GetService<UiServiceInterface>();
        //if (!uiService_)
        //    return;

		qWin_ = dynamic_cast<UiMainWindow*>(owner_->GetFramework()->Ui()->MainWindow());
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
		configWindow_ = new ViewDialog(views);
        owner->GetFramework()->Ui()->MainWindow()->AddMenu("Window");
		//uiService_->AddExternalMenu("Window");

		//Insert New Action

		//QAction* newAction = new QAction(tr("New"), this);
        QAction* newAction = owner->GetFramework()->Ui()->MainWindow()->AddMenuAction("Window", "New", QIcon(), 99);
		connect(newAction, SIGNAL(triggered()), SLOT(NewViewWindow()));
		//uiService_->AddExternalMenuAction(newAction, "New", "Window", 0, 99);

		//Insert Configure Action
		//QAction* open = new QAction(tr("Configure"), this);
        QAction* open = owner->GetFramework()->Ui()->MainWindow()->AddMenuAction("Window", "Configure", QIcon(), 98);
		connect(open, SIGNAL(triggered()),configWindow_, SLOT(show()));
		//uiService_->AddExternalMenuAction(open, "Configure", "Window", 0, 98);

		//Add separator
		//QAction* separator = new QAction(tr(""), this);
        QAction* separator = owner->GetFramework()->Ui()->MainWindow()->AddMenuAction("Window", "", QIcon(), 97);
		separator->setSeparator(true);
		//uiService_->AddExternalMenuAction(separator, "separator", "Window", 0,  97);

		//Create the Action Group and insert all views
		actionGroup_=new QActionGroup(this);
		connect(actionGroup_, SIGNAL(triggered(QAction*)), SLOT(ActionChanged(QAction*)));

		//previous_ = new QAction(tr("Previous"), this);
        previous_ = owner->GetFramework()->Ui()->MainWindow()->AddMenuAction("Window", "Previous", QIcon(), 50);
		actionGroup_->addAction(previous_);
		//uiService_->AddExternalMenuAction(previous_, tr("Previous"), "Window", 0, 50, true);
		previous_->setVisible(false);

		//QAction* hide = new QAction(tr("Hide"), this);
        QAction* hide = owner->GetFramework()->Ui()->MainWindow()->AddMenuAction("Window", "Hide");
		actionGroup_->addAction(hide);
		//uiService_->AddExternalMenuAction(hide, tr("Hide"), "Window", 0, 50, true);

		QListIterator<QString> i(views);
		while(i.hasNext()){
			QString name=i.next();
			//actionGroup_->addAction(new QAction(name,this));
            actionGroup_->addAction(owner->GetFramework()->Ui()->MainWindow()->AddMenuAction("Window", name));
		}

		QListIterator<QAction*> q(actionGroup_->actions());
		while(q.hasNext()){
			QAction* a=q.next();
			a->setCheckable(true);
			//uiService_->AddExternalMenuAction(a, a->text(), "Window", 0, 50, true);
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
		//QList<QString> widgets = uiService_->GetAllWidgetsNames();
        QMap<UiWidget*, QDockWidget*> external_wid = owner_->GetExternalWidgets();
        QList<UiWidget*> widgets = external_wid.keys();

		QListIterator<UiWidget*> i(widgets);
		while(i.hasNext()){
            QDockWidget* widget = external_wid.value(i.next());
            widget->hide();
		}
	}
	void ViewManager::TogglePreviousView(bool save)
	{
		if(save){
			previous_->setVisible(true);
    		StoreViewSettings("Previous");
 		}else{
            QSettings settings(QSettings::IniFormat, QSettings::UserScope, APPLICATION_NAME, "configuration/ConfigurationViews");
			settings.remove("Previous");
			previous_->setVisible(false);
		}
	}

    void ViewManager::StoreViewSettings(const QString &viewname)
    {
        QSettings settings(QSettings::IniFormat, QSettings::UserScope, APPLICATION_NAME, "configuration/ConfigurationViews");

        settings.beginGroup(viewname);

        //Delete the old widgets
        QStringList keys = settings.allKeys();
        QListIterator<QString> key(keys);
        while(key.hasNext())
            settings.remove(key.next());

        settings.setValue("win_state", qWin_->saveState());

        QMap<UiWidget*, QDockWidget*> external_wid = owner_->GetExternalWidgets();
        QList<UiWidget*> widgets = external_wid.keys();
        QListIterator<UiWidget*> i(widgets);

        while(i.hasNext()){
            UiWidget* uiWidget = i.next();
            //QDockWidget* dockWidget = external_wid.value(uiWidget);
            //if(dynamic_cast<QDockWidget*>(widget))
            //	widget = dynamic_cast<QDockWidget*>(widget)->widget();
            QVariant dynamic_property = uiWidget->property("dynamic");
            if(dynamic_property.isValid() && dynamic_property.toBool()){
                QVariant type_property = uiWidget->property("type");
                if (type_property.isValid()) {
                    settings.beginGroup(uiWidget->windowTitle());
                    settings.setValue(QString("type"), type_property);
                    QList<QByteArray> properties = uiWidget->dynamicPropertyNames();
                    QListIterator<QByteArray> p(properties);
                    while(p.hasNext()){
                        QString s = p.next();
                        if(s.startsWith("DP_"))
                            settings.setValue(s, uiWidget->property(s.toAscii()));
                    }
                    settings.endGroup();
                }
            }
        }
        settings.endGroup();
    }

	void ViewManager::ShowView(const QString &name)
	{
		//Restore values
		QSettings settings(QSettings::IniFormat, QSettings::UserScope, APPLICATION_NAME, "configuration/ConfigurationViews");
		if(settings.childGroups().contains(name)){
			settings.beginGroup(name);

			//QList<QString> currentWidgets = uiService_->GetAllWidgetsNames();
            QMap<UiWidget*, QDockWidget*> external_wid = owner_->GetExternalWidgets();
            QList<UiWidget*> currentWidgets = external_wid.keys();
            QList<QString> currentWidgetsNames;
			QList<QString> widgets = settings.childGroups();
			QListIterator<QString> i(widgets);
			QListIterator<UiWidget*> k(currentWidgets);
			QSet<QString> qset = QSet<QString>::fromList(widgets);

			//Delete all dynamic widgets that aren't in view
			while(k.hasNext()){
                UiWidget* uiWidget = k.next();
                currentWidgetsNames.append(uiWidget->windowTitle());
                QVariant dynamic_property = uiWidget->property("dynamic");
                if(!qset.contains(uiWidget->windowTitle()) && dynamic_property.isValid() && dynamic_property.toBool())
					//TODO!uiService_->HideWidget(widgetName);
                    external_wid.value(uiWidget)->hide();
			}

			//Create all dynamic widgets in view
			while(i.hasNext()){
				QString widgetName = i.next();
				if(!currentWidgetsNames.contains(widgetName)){
					QString type="";
					QStringList properties;
					settings.beginGroup(widgetName);
					QStringList keys = settings.childKeys();
					QListIterator<QString> j(keys);
					while(j.hasNext()){
						QString prop = j.next(); //Only has one element
						if(prop=="type")
							type=settings.value(prop).toString();
						else
                        {   properties.append(prop);
                            properties.append(settings.value(prop).toString());
                        }
					}
					settings.endGroup();
                    if (!type.isEmpty())
                    {
                        UiWidgetFactoryPtr factory = owner_->GetFramework()->Ui()->GetUiWidgetFactory(type);
                        if (factory)
                            factory->CreateWidget(widgetName, properties);
                    }
				}
			}
			//Restore the state of main window
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
			//QAction* a = new QAction(name,this);
            QAction* a = owner_->GetFramework()->Ui()->MainWindow()->AddMenuAction("Window", name);
			a->setCheckable(true);
			actionGroup_->addAction(a);
			//uiService_->AddExternalMenuAction(a, a->text(), "Window", 0, 50, true);
			a->setChecked(true);
		}
		
        StoreViewSettings(name);

		//settings.beginGroup(name);

		////Delete the old widgets
		//QStringList keys = settings.allKeys();
		//QListIterator<QString> key(keys);
		//while(key.hasNext())
		//	settings.remove(key.next());
		//	
		//settings.setValue("win_state", qWin_->saveState());

		////QList<QString> widgets = uiService_->GetAllWidgetsNames();
  //      QMap<UiWidget*, QDockWidget*> external_wid = owner_->GetExternalWidgets();
  //      QList<UiWidget*> widgets = external_wid.keys();
		//QListIterator<UiWidget*> i(widgets);

		//while(i.hasNext()){
		//	UiWidget* uiWidget = i.next();
		//	//QWidget* widget = uiService_->GetWidget(nameWidget);
  //          //QWidget* widget = external_wid.value(nameWidget);

		//	if(dynamic_cast<QDockWidget*>(widget))
		//		widget = dynamic_cast<QDockWidget*>(widget)->widget();
		//	if(widget->property("dynamic").isValid()){
		//		settings.beginGroup(nameWidget);
		//		QList<QByteArray> properties = widget->dynamicPropertyNames();
		//		QListIterator<QByteArray> p(properties);
		//		while(p.hasNext()){
		//			QString s = p.next();
		//			if(s.startsWith("DP_"))
		//				 settings.setValue(s, widget->property(s.toAscii()));
		//		}
		//		settings.endGroup();
		//	}
		//}
		//settings.endGroup();
		
		configWindow_->UpdateViews(settings.childGroups());
	}

	void ViewManager::DeleteView(const QString &name)
	{
		QSettings settings(QSettings::IniFormat, QSettings::UserScope, APPLICATION_NAME, "configuration/ConfigurationViews");
		
		QStringList groups = settings.childGroups();
		if(groups.contains(name)){
			QListIterator<QAction*> q(actionGroup_->actions());
			//uiService_ = owner_->GetFramework()->GetService<UiServiceInterface>();
			while(q.hasNext()){
				QAction* a=q.next();
				if(a->text()==name /*&& uiService_*/) //Check if uiservice is still available
                    owner_->GetFramework()->Ui()->MainWindow()->RemoveMenuAction(a);
					//todo!uiService_->RemoveExternalMenuAction(a);
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
		if(actionGroup_->checkedAction()->text()=="Previous"){
			ShowView(actionGroup_->checkedAction()->text());
			TogglePreviousView(false);
		}else if(actionGroup_->checkedAction()->text()=="Hide"){
			TogglePreviousView(true);
			HideView();
		}else{
			TogglePreviousView(true);
			ShowView(actionGroup_->checkedAction()->text());
		}

	}
}
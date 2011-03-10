// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "ChangeThemeWidget.h"
#include "UiDarkBlueStyle.h"
// #include "UiProxyStyle.h"

#include <QApplication>
#include <QFontDatabase>
#include <QDebug>
#include <QStyleFactory>
#include <QSettings>

#include "MemoryLeakCheck.h"

namespace CoreUi
{
    ChangeThemeWidget::ChangeThemeWidget(QObject* settings_widget) : 
        QWidget()
    {
        setupUi(this);
        connect(settings_widget, SIGNAL(SaveSettingsClicked()), this, SLOT(ChangeTheme()));
//        connect(settings_widget, SIGNAL(CancelClicked()), settings_widget, SLOT(CancelClicked()));
        
        comboBox_changeTheme->addItem(QString::fromStdString("Naali dark blue"));
        comboBox_changeTheme->addItems(QStyleFactory::keys());

		QSettings settings(QSettings::IniFormat, QSettings::UserScope, APPLICATION_NAME, "configuration/UiExternalSettings");
		QString theme = settings.value("default_theme_used", QString("")).toString();
		if (theme == "") {
			//Use the default one. Typically they include "windows", "motif", "cde", "plastique" and "cleanlooks"
			QApplication::setPalette(QApplication::style()->standardPalette());
			int i = 1;
			foreach (QString str, QStyleFactory::keys()) {
				if (str.toLower().replace(" ","") == QApplication::style()->objectName())
					comboBox_changeTheme->setCurrentIndex(i);
				i++;
			}
/*
			QStringList aux_list = QStyleFactory::keys();
			aux_list.
			comboBox_changeTheme->setCurrentIndex(QStyleFactory::keys().indexOf(QApplication::style()->objectName()));
			
			if (QStyleFactory::keys().contains("Windows"))
				theme = "Windows";
			else
				theme = "naali dark blue";
				*/
		}
		else
		{
			comboBox_changeTheme->setCurrentIndex(settings.value("default_theme_index_used", 1).toInt());
			//Default theme
			if (theme.toLower() == "naali dark blue")
			{
				QApplication::setStyle(new UiServices::UiDarkBlueStyle());
				QFontDatabase::addApplicationFont("./media/fonts/FACB.TTF");
				QFontDatabase::addApplicationFont("./media/fonts/FACBK.TTF");
			} 
			else
			{
				QApplication::setStyle(QStyleFactory::create(theme));
			}
			QApplication::setPalette(QApplication::style()->standardPalette());
		}
    }

    ChangeThemeWidget::~ChangeThemeWidget()
    {
    }

    void ChangeThemeWidget::ChangeTheme()
    {        
        QString theme = comboBox_changeTheme->currentText();

        //If theme not changed, return.
        if (!currentTheme.isEmpty() && theme == currentTheme)
            return;
        currentTheme = theme;

        //
		if (theme.toLower() == "naali dark blue") 
        {
			QApplication::setStyle(new UiServices::UiDarkBlueStyle());
            QFontDatabase::addApplicationFont("./media/fonts/FACEB.TTF");
        } 
        else
        {
            QFontDatabase::removeAllApplicationFonts();
            QApplication::setStyle(QStyleFactory::create(theme));
            QApplication::setPalette(QApplication::style()->standardPalette());
        }
        QApplication::setPalette(QApplication::style()->standardPalette());
		
		QSettings settings(QSettings::IniFormat, QSettings::UserScope, APPLICATION_NAME, "configuration/UiExternalSettings");
		settings.setValue("default_theme_used", theme.toLower());
		settings.setValue("default_theme_index_used", comboBox_changeTheme->currentIndex());
    }


}

// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "ChangeThemeWidget.h"
#include "UiDarkBlueStyle.h"
#include "UiProxyStyle.h"

#include <QApplication>
#include <QFontDatabase>
#include <QDebug>
#include "MemoryLeakCheck.h"

namespace CoreUi
{
    ChangeThemeWidget::ChangeThemeWidget(QObject* settings_widget) : 
        QWidget()
    {
        setupUi(this);
        connect(settings_widget, SIGNAL(SaveSettingsClicked()), this, SLOT(ChangeTheme()));
        connect(settings_widget, SIGNAL(CancelClicked()), settings_widget, SLOT(CancelClicked()));

        comboBox_changeTheme->addItem(QString::fromStdString("Default style"));
        comboBox_changeTheme->addItem(QString::fromStdString("Dark blue"));
       
    }

    ChangeThemeWidget::~ChangeThemeWidget()
    {}

    void ChangeThemeWidget::ChangeTheme()
    {        
        QString theme = comboBox_changeTheme->currentText();

        //If theme not changed, return.
        if (!currentTheme.isEmpty() && theme == currentTheme)
            return;

        if (currentTheme.isEmpty())        
            currentTheme = theme;        

        if (theme == "Dark blue") {
            QApplication::setStyle(new UiServices::UiDarkBlueStyle());
            QFontDatabase::addApplicationFont("./media/fonts/FACB.TTF");
            QFontDatabase::addApplicationFont("./media/fonts/FACBK.TTF");

        } else {
            QApplication::setStyle(new UiServices::UiProxyStyle());
        }
        
        QApplication::setPalette(QApplication::style()->standardPalette());
         
    }


}
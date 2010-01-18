// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_SettingsWidget_h
#define incl_UiModule_SettingsWidget_h

#include "ui_SettingsWidget.h"

#include <QWidget>

namespace CoreUi
{
    class SettingsWidget : public QWidget
    {
    
    Q_OBJECT
    
    public:
        SettingsWidget();
        virtual ~SettingsWidget();

    public slots:
        void AddWidget(QWidget *widget, const QString &tab_name);

    private:
        Ui::SettingsWidget setting_ui_;
    };
}

#endif // incl_UiModule_SettingsWidget_h
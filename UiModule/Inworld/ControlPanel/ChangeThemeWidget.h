// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_ChangeThemeWidget_h
#define incl_UiModule_ChangeThemeWidget_h

#include <QWidget>

#include "UiProxyWidget.h"
#include "ui_ChangeThemeWidget.h"

namespace CoreUi
{
    class ChangeThemeWidget : public QWidget, private Ui::ChangeTheme
    {
        Q_OBJECT

        public:
            ChangeThemeWidget(QObject* settings_widget);
            virtual ~ChangeThemeWidget();

        public slots:
            void ChangeTheme();

        private:
            QString currentTheme;

    };

}

#endif
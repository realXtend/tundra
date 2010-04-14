// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_LanguageWidget_h
#define incl_UiModule_LanguageWidget_h

#include <QWidget>
#include <QStringList>
#include <QString>
#include <QMap>
#include <QListWidgetItem>

#include "Inworld/View/UiProxyWidget.h"
#include "ui_LanguageWidget.h"

namespace CoreUi
{
    class LanguageWidget : public QWidget, private Ui::Language
    {
        Q_OBJECT

        public:
            LanguageWidget(QObject* settings_widget);
            virtual ~LanguageWidget();

        public slots:
            void ExportSettings();
            void ItemPressed(QListWidgetItem* item);
            void CancelClicked();

        signals:
            void LanguageSelected(const QString& file);

        private:
        
            QString LanguageName(QString qmFile);
            QStringList findQmFiles();
            
            QMap<QListWidgetItem*, QString> qmFileForCheckBoxMap;
         

    };

}

#endif
// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Input_BindingWidget_h
#define incl_Input_BindingWidget_h

#include "ui_BindingWidget.h"

#include <QGraphicsProxyWidget>

namespace Input
{
    class KeyDataManager;

    class BindingWidget : public QWidget, private Ui::BindingWidget
    {

    Q_OBJECT

    public:
        BindingWidget::BindingWidget(KeyDataManager *key_data_manager, QObject *settings_object);

    public slots:
        void UpdateContent(QMap<QString, QList<QVariant> > key_map);

    private slots:
        void ExportSettings();
        void RestoreDefaultBindings();

        QLineEdit *GetLineEditForName(QString name);
    
    private:
        KeyDataManager *key_data_manager_;

    };
}

#endif
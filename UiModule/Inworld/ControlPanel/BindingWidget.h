// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Input_BindingWidget_h
#define incl_Input_BindingWidget_h

#include "ui_BindingWidget.h"
#include "KeyBindings.h"

#include <QGraphicsProxyWidget>
#include <QStringList>

namespace CoreUi
{
    class BindingWidget : public QWidget, private Ui::BindingWidget
    {

    Q_OBJECT

    public:
        BindingWidget::BindingWidget(QObject *settings_object);

    public slots:
        void UpdateContent(Foundation::KeyBindings *bindings);

    private slots:
        void ExportSettings();
        void RestoreBindingsToDefault();

        QLineEdit *GetLineEditForName(QString name);

    signals:
        void KeyBindingsUpdated(Foundation::KeyBindings*);
        void RestoreDefaultBindings();

    };
}

#endif
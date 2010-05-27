
#ifndef incl_ECEditorModule_MultiEditWidget_h
#define incl_ECEditorModule_MultiEditWidget_h

#include <QPushButton>
#include <QLabel>

class QDialog;
class QListWidget;
class QLineEdit;
class QPushButton;
class QPushButton;
class QLayout;
class QInputDialog;

namespace ECEditor
{
    class MultiSelectButton: public QPushButton
    {
        Q_OBJECT
    public:
        MultiSelectButton(QWidget *parent = 0):
            QPushButton(parent)
        {
        }
        ~MultiSelectButton()
        {
        }

    signals:
        void ButtonClicked();

    protected:
        void mousePressEvent(QMouseEvent *event);
    };

    class MultiEditWidget: public QWidget
    {
        Q_OBJECT
    public:
        MultiEditWidget(QWidget *parent = 0);
        ~MultiEditWidget();

        void SetLabelText(const QString &text);

    signals:
        void ButtonClicked();

    private:
        QPushButton     *button_;
        QLabel          *text_label_;
    };
}

#endif
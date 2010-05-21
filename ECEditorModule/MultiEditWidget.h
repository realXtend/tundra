
#ifndef incl_ECEditorModule_MultiEditWidget_h
#define incl_ECEditorModule_MultiEditWidget_h

#include <QPushButton>
#include <QDialog>

class QDialog;
class QListWidget;
class QLineEdit;
class QPushButton;
class QPushButton;
class QLayout;
class QInputDialog;

namespace ECEditor
{
    //! Multiedit dialog is used when user has selected multiple EC components that
    //! contains same type of attributes, and he/she wants to change them all in one
    //! pick. Multiedit dialog will treat all attribute values as strings and will return
    //! final pick in string format, it's up to user to convert the attribute value back to
    //! its original form.
    /*class MultiEditDialog: public QDialog
    {
        Q_OBJECT
    public:
        MultiEditDialog(QWidget *parent = 0, Qt::WindowFlags flag = 0);
        ~MultiEditDialog();

        //! Will send all attribute values that we want to be show on list. Should be called before the
        //! dialog is displayed.
        //! @param attributeValues contains all attribute values in string vector format.
        void AddAttributeValues(QVector<QString> attributeValues);
        //! Get lineEdit's text value as final result. Should be called when accepted signal is emitted.
        QString GetAttributeValue() const;

    public slots:
        //! Called when user picks one of the attribute value from the list widget.
        void UpdateLineEditText();

    private:
        QListWidget *list_;
        QLineEdit   *lineEdit_;
        QPushButton *okButton_;
        QPushButton *cancelButton_;
        QLayout     *verticalLayout_;
    };*/

    class MultiEditWidget: public QPushButton
    {
        Q_OBJECT
    public:
        MultiEditWidget(QWidget *parent = 0);
        ~MultiEditWidget();

        void SetAttributeValues(QStringList attributeValues);
        QString GetFinalResult() const {return selectedValue_;}

    public slots:
        void CreateMultiSelectDialog();
        void GetFinalResultFromDialog(const QString &select);

    signals:
        void ValueSelected(const QString &select);

    private:
        QInputDialog    *dialog_;
        QStringList     attributeValues_;
        QString         selectedValue_;
    };
}

#endif
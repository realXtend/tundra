/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   ScriptDialogWidget.cpp
 *  @brief  Dialog Widget started by llDialog script command at server.
 */

#ifndef incl_RexLogicModule_ScriptDialogWidget_h
#define incl_RexLogicModule_ScriptDialogWidget_h

#include "ScriptDialogRequest.h"

#include <QPushButton>
#include <QLineEdit>

class QWidget;

namespace RexLogic
{
    /**
     * Button for ScriptDialog selection. It emit Clicked event with 
     *
     */
    class SelectionButton : public QPushButton
    {
        Q_OBJECT

    public:
        //! Constructor
        //! @param id Id to identify button later when click event is triggered
        SelectionButton(QWidget *parent, const char * name = 0, QString id="" );

        //! @return id given to constructor
        QString GetId();

    private slots:
        void OnClicked();

    private:
        QString id_;

    signals:
        void Clicked(QString id);
    };

    /**
     * Dialog for presenting 1 to 12 labeled buttons and ignore button.
     * Used for requesting response for ScriptDialog network event emited by
     * llDialog script command on server side.
     *
     */
    class ScriptDialogWidget : public QWidget
    {
        Q_OBJECT

    public:
        /// Constructor.
        ScriptDialogWidget(ScriptDialogRequest &request, QWidget *parent = 0);

        /// Destructor.
        virtual ~ScriptDialogWidget();

    protected:
        void hideEvent(QHideEvent *hide_event);
        void InitWindow(ScriptDialogRequest &request);
        void showEvent(QShowEvent* show_event);

    private:
        QWidget *widget_;
        ScriptDialogRequest request_;
        QString text_value_;
        QLineEdit *text_input_;
        //UiServices::UiProxyWidget *proxyWidget_;

    private slots:
        void OnButtonPressed(QString id);
        void OnIgnorePressed();

    signals:
        void OnClosed(int channel, QString answer);
    };
}

#endif // incl_RexLogic_ScriptDialogWidget_h

// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_WebLoginWidget_h
#define incl_UiModule_WebLoginWidget_h

#include "ui_WebLoginWidget.h"

namespace CoreUi
{
    namespace Classical
    {
        class WebLoginWidget : public QWidget, private Ui::WebLogin
        {
            
        Q_OBJECT

        public:
            WebLoginWidget(QWidget *parent);

        private slots:
            void InitWidget();
            void ConnectSignals();
        
        public slots:
            void GoToUrl();
            void GoToUrl(bool checked);
            void LoadStarted();
            void UpdateUi(int progress);
            void ProcessPage(bool success);
            void LoadUrl(QUrl url);

        private:
            QString address_;

        signals:
            void WebLoginInfoRecieved(QWebFrame *);
            void WebLoginUrlRecived(QString);
        };
    }
}

#endif // incl_UiModule_WebLoginWidget_h

// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_WebLogin_h
#define incl_WebLogin_h

#include <QtGui>
#include <QWidget>
#include <QWebView>
#include <QProgressBar>

namespace CoreUi
{
    class WebLogin : public QWidget
    {
        Q_OBJECT

    public:
        WebLogin(QWidget *parent, QString address);
        virtual ~WebLogin();

    public slots:
        void GoToUrl();
        void GoToUrl(bool checked);
        void LoadStarted();
        void UpdateUi(int progress);
        void ProcessPage(bool success);

    private:
        void InitWidget();
        void ConnectSignals();

        QWidget *widget_;
        QWebView *webView_;
        QProgressBar *progress_bar_;
        QLabel *statusLabel;
        QComboBox *combobox_address_;
        QPushButton *refresh_button_;
        QPushButton *back_button_;
        QPushButton *forward_button_;
        QPushButton *stop_button_;
        QPushButton *go_button_;
        QVBoxLayout *layout_;
        QString address_;

    signals:
        void WebLoginInfoRecieved(QWebFrame *);
    };
}

#endif

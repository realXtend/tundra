// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "WebLoginWidget.h"

#include <QFile>
#include <QLineEdit>
#include <QWebFrame>
#include <QUrl>

#include "MemoryLeakCheck.h"
#include "NetworkAccessManager.h"

namespace CoreUi
{
    namespace Classical
    {
        WebLoginWidget::WebLoginWidget(QWidget *parent) : 
            QWidget(parent)
        {
            InitWidget();
            ConnectSignals();
        }

        void WebLoginWidget::InitWidget()
        {
            // Read default url from file
            QFile confFile("./data/default_login.ini");
            if (!confFile.open(QIODevice::ReadOnly | QIODevice::Text))
                return;
            address_ = confFile.readLine();
            confFile.close();

            // Init UI
            setupUi(this);

            comboBox_Address->setEditText(address_);
            
            pushButton_Back->setIcon(QIcon("./data/ui/images/arrow_left_48.png"));
            pushButton_Back->setIconSize(QSize(20, 20));
            pushButton_Forward->setIcon(QIcon("./data/ui/images/arrow_right_48.png"));
            pushButton_Forward->setIconSize(QSize(20, 20));
            pushButton_Stop->setIcon(QIcon("./data/ui/images/cross_48.png"));
            pushButton_Stop->setIconSize(QSize(20, 20));
            pushButton_Stop->setEnabled(false);
            pushButton_Refresh->setIcon(QIcon("./data/ui/images/refresh_48.png"));
            pushButton_Refresh->setIconSize(QSize(20, 20));
            pushButton_Go->setIcon(QIcon("./data/ui/images/arrow_right_green_48.png"));
            pushButton_Go->setIconSize(QSize(20, 20));
        }

        void WebLoginWidget::ConnectSignals()
        {
            QNetworkAccessManager *oldManager = webView->page()->networkAccessManager();
            NetworkAccessManager *newManager = new NetworkAccessManager(oldManager, webView);
            webView->page()->setNetworkAccessManager(newManager);

            connect(newManager, SIGNAL( WebLoginUrlRecived(QUrl) ), this, SLOT( LoadUrl(QUrl) ));

            // Buttons
            connect(pushButton_Back, SIGNAL( clicked() ), webView, SLOT( back() ));
            connect(pushButton_Forward, SIGNAL( clicked() ), webView, SLOT( forward() ));
            connect(pushButton_Stop, SIGNAL( clicked() ), webView, SLOT( stop() ));
            connect(pushButton_Refresh, SIGNAL( clicked() ), webView, SLOT( reload() ));
            connect(pushButton_Go, SIGNAL( clicked(bool) ), this, SLOT( GoToUrl(bool) ));
            
            // Addressbar
            connect(comboBox_Address->lineEdit(), SIGNAL( returnPressed() ), this, SLOT( GoToUrl() ));
            
            // Webview
            connect(webView, SIGNAL( loadStarted() ), this, SLOT( LoadStarted() ));
            connect(webView, SIGNAL( loadProgress(int) ), this, SLOT( UpdateUi(int) ));
            connect(webView, SIGNAL( loadFinished(bool) ), this, SLOT( ProcessPage(bool) ));

            webView->setUrl(QUrl(address_));
        }

        void WebLoginWidget::GoToUrl()
        {
            GoToUrl(true);
        }

        void WebLoginWidget::GoToUrl(bool checked)
        {
            webView->setUrl(QUrl(comboBox_Address->lineEdit()->text()));
        }

        void WebLoginWidget::LoadStarted()
        {
            pushButton_Stop->setEnabled(true);
            label_Status->setText("Loading page...");
            progressBar_Status->show();
        }

        void WebLoginWidget::UpdateUi(int progress)
        {
            if (progressBar_Status)
                progressBar_Status->setValue(progress);
        }

        void WebLoginWidget::ProcessPage(bool success)
        {
            if (success)
            {
                // Update GUI
                pushButton_Stop->setEnabled(false);
                address_ = webView->url().toString();
                comboBox_Address->lineEdit()->setText(address_);
                //setWindowTitle(webView_->page()->mainFrame()->title().append(" - realXtend Naali web browser"));
                if (comboBox_Address->findText(address_, Qt::MatchFixedString) == -1)
                    comboBox_Address->addItem(address_);
                label_Status->setText("Done");
                progressBar_Status->hide();

                // Do actual HTML page processing if this was a login success page...
                if (webView->page()->mainFrame()->title() == "LoginSuccess")
                {
                    int pos1, pos2;
                    QString entry_point_url;
                    QString returnValue = webView->page()->mainFrame()->evaluateJavaScript("ReturnSuccessValue()").toString();

                    pos1 = returnValue.indexOf(QString("http://"), 0);
                    pos2 = returnValue.indexOf(QString("?"), 0);
                    entry_point_url = returnValue.mid(pos1, pos2-pos1);
                    //emit WebLoginInfoRecieved(webView->page()->mainFrame());
                    emit WebLoginUrlRecived(entry_point_url);
                }
            }
        }

        void WebLoginWidget::LoadUrl(QUrl url)
        {
            if (url.scheme() == "cablebeach")
            {
                QString entry_point_url;
                QString urlString = url.toString().replace("cablebeach://", "http://");
                emit WebLoginUrlRecived(urlString);
            }
        }
    }
}

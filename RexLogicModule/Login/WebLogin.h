// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_WebLogin_h
#define incl_WebLogin_h

#include <QtGui>
#include <QWidget>
#include <QWebView>
#include <QProgressBar>

namespace RexLogic
{
	class RexLogicModule;
	class WebLogin 
		: public QWidget
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
		QProgressBar *progressBar;
		QLabel *statusLabel;
		QComboBox *comboBoxAddress;
		QPushButton *refreshButton;
		QPushButton *backButton;
		QPushButton *forwardButton;
		QPushButton *stopButton;
		QPushButton *goButton;
		QVBoxLayout *layout_;
		QString address_;

	signals:
		void WebLoginInfoRecieved(QWebFrame *);

	};
}

#endif
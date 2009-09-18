// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexWebLogin_h
#define incl_RexWebLogin_h

#include <QtGui>
#include <QWidget>
#include <QWebView>
#include <QProgressBar>

namespace RexLogic
{
	class RexLogicModule;
	class RexWebLogin 
		: public QWidget
	{
		Q_OBJECT

	public:
		RexWebLogin(QWidget *parent, QString address);
		virtual ~RexWebLogin();

	public slots:
		void goToUrl();
		void goToUrl(bool checked);
		void loadStarted();
		void updateUi(int progress);
		void processPage(bool success);

	private:
		void showEvent(QShowEvent *showEvent);
		void initWidget();
		void connectSignals();

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
		void loginProcessed(QString);

	};
}

#endif
// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexWebLogin_h
#define incl_RexWebLogin_h

#include <QtGui>
#include <QWidget>
#include <QWebView>

namespace RexLogic
{
	class RexLogicModule;
	class RexWebLogin 
		: public QWidget
	{

	public:
		RexWebLogin(QWidget *parent);
		virtual ~RexWebLogin();

	private:
		void initWidget();
		void connectSignals();

		QWidget *widget_;
		QWebView *webView_;
		QVBoxLayout *layout_;

	};
}

#endif
#include "cbloginwidget.h"

CBLoginWidget::CBLoginWidget(QWidget *parent, Qt::WFlags flags)
	: QFrame(parent)
{
	ui.setupUi(this);

	view = new QGraphicsView(this);
	scene = new QGraphicsScene();
	
	setProperties();
	initWidgets();
	showLoginPanel();
}

CBLoginWidget::~CBLoginWidget()
{
	delete scene;
}

void CBLoginWidget::setProperties()
{
  // Read config
    
  // Note: QString should change automagically separator to correct version. Depending of OS.   
  configmanager.setConfigFilePath(QString("media/cblogin/UserIdentities.xml"));
	
  if ( !configmanager.readConfig() )
		showMessageDialog("Parsing UserIdentities.xml failed");

	// Scene
  	QBrush b = QBrush( QPixmap("media/cblogin/background.png") );
	scene->setBackgroundBrush( b );
	scene->setStickyFocus(true);

	// View
	view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	view->setScene(scene);
	ui.verticalLayout->addWidget(view);

	// Progressbar show state
	loadSplashState = 0;
}

void CBLoginWidget::initWidgets()
{
	// Login panel
	loginpanel = new LoginPanel();
	loginpanelProxy = new CustomProxy(0, Qt::Widget);
	loginpanelProxy->setWidget(loginpanel);
	loginpanelProxy->setGeometry( QRectF(0, 0, 0, 0) );
	QObject::connect( loginpanel->ui->pushButtonLogin, SIGNAL( clicked() ), SLOT( showWebView() ) );

	// Webview
	loginwebview = new LoginWebView(this, view);
	loginwebviewProxy = new CustomProxy(0, Qt::Widget);
	loginwebviewProxy->setWidget(loginwebview);
	loginwebviewProxy->setGeometry( QRectF(0, 0, 0, 0) );
	QObject::connect(loginwebview->ui->webView, SIGNAL( loadFinished(bool) ), SLOT( processLogin(bool) ));
	QObject::connect(loginwebview->ui->pushButtonCancel, SIGNAL( clicked() ), SLOT( showLoginPanel() ));
}

void CBLoginWidget::showMessageDialog(QString message)
{
	QMessageBox *dialog = new QMessageBox(this);
	dialog->setIcon(QMessageBox::Information);
	dialog->setText( message );
	dialog->show();
}

void CBLoginWidget::resizeEvent(QResizeEvent *rEvent)
{
	if ( loadSplashState != 0 )
	{
		QSize mysize = rEvent->size();
		loginwebview->loadSplash->setGeometry( (mysize.width() / 2) - 180, (mysize.height() / 2) - 50, 0, 0 );
	}
	QWidget::resizeEvent(rEvent);
}


////////////////////////////////////////////
//             PUBLIC SLOTS               //
////////////////////////////////////////////

void CBLoginWidget::showLoginPanel()
{
	// Load configs to drop down menus, if there is changes
	if ( configmanager.getWorldList()->length() > loginpanel->ui->comboBoxWorlds->count() - 1 )
	{
		// If there is changes in the QStringList then we empty combobox
		loginpanel->ui->comboBoxWorlds->clear();
		loginpanel->ui->comboBoxWorlds->addItem("");
		loginpanel->setComboBoxItems("worlds", configmanager.getWorldList());
	}

	if ( configmanager.getOpenIDList()->length() > loginpanel->ui->comboBoxOpenIDs->count() -1 )
	{
		// If there is changes in the QStringList then we empty combobox
		loginpanel->ui->comboBoxOpenIDs->clear();
		loginpanel->ui->comboBoxOpenIDs->addItem("");
		loginpanel->setComboBoxItems("openids", configmanager.getOpenIDList());
	}
	scene->removeItem(loginwebviewProxy);
	scene->addItem(loginpanelProxy);
	loadSplashState = 0;
}

void CBLoginWidget::showWebView()
{
	if ( loginpanel->ui->lineEditWorld->text().length() != 0 && loginpanel->ui->lineEditOpenID->text().length() != 0 )
	{
		scene->removeItem(loginpanelProxy);
		scene->addItem(loginwebviewProxy);
		loginwebview->loadAutoLoginPage();
		loadSplashState = 1;
	}
	else
	{
		showMessageDialog(QString("URL fields cannot be empty"));
	}
}

void CBLoginWidget::processLogin(bool success)
{
	QString url = loginwebview->ui->webView->page()->mainFrame()->url().toString(); // debug...
	if ( success )
	{
		QString pageTitle = loginwebview->ui->webView->page()->mainFrame()->title();
		if ( pageTitle == "AutomatedLogin" ) 
		{
			loginwebview->ui->webView->page()->mainFrame()->addToJavaScriptWindowObject(loginpanel->objectName(), loginpanel);
			loginwebview->ui->webView->page()->mainFrame()->evaluateJavaScript("loginWithData()");
		} 
		else if ( pageTitle == "LoginSuccess") 
		{
			// Check if user wants to save entries to config
			if ( loginpanel->ui->checkBoxSave->isChecked() )
			{
				if ( !configmanager.writeCongif(loginpanel->ui->lineEditWorld->text(), loginpanel->ui->lineEditOpenID->text()) ) 
					showMessageDialog("Writing to UserIdentities.xml failed");
			}
			QVariant returnValue;
			returnValue = loginwebview->ui->webView->page()->mainFrame()->evaluateJavaScript("ReturnSuccessValue()");
			emit( loginProcessed(returnValue.toString()) );
			this->close();
		}
	}
	else 
	{
		showMessageDialog(QString("Could not connect to World Server, please correct the URL"));
		showLoginPanel();
	}
}

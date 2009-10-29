// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "RexLoginWindow.h"
#include "RexLogicModule.h"
#include "RexServerConnection.h"
#include "QtModule.h"
#include "UICanvas.h"

#include <QFile>
#include <QPushButton>
#include <QLabel>
#include <QString>
#include <QtUiTools>
#include <QTabWidget>
#include <QLineEdit>
#include <QGraphicsScene>
#include <QGraphicsView>

#include "Poco/URI.h"

#include "MemoryLeakCheck.h"

namespace RexLogic
{

RexLoginWindow::RexLoginWindow(Foundation::Framework* framework, RexLogicModule *module) :
    framework_(framework),
    rex_logic_(module),
    login_widget_(0),
    logout_button_(0),
    quit_button_(0),
    webLogin(0)
{
    InitLoginWindow();
}

RexLoginWindow::~RexLoginWindow()
{
    // These are own by canvases so they deal cleaning operations.
    login_widget_ = 0;
    logout_button_ = 0;
    quit_button_ = 0;

    Foundation::ModuleSharedPtr qt_module = framework_->GetModuleManager()->GetModule("QtModule").lock();
    QtUI::QtModule *qt_ui = dynamic_cast<QtUI::QtModule*>(qt_module.get());

    // If this occurs, we're most probably operating in headless mode.
    if (qt_ui != 0)
    {
        qt_ui->DeleteCanvas(canvas_->GetID());
        qt_ui->DeleteCanvas(screen_canvas_->GetID());
    }

    delete webLogin;
}

void RexLoginWindow::InitLoginWindow()
{
    boost::shared_ptr<QtUI::QtModule> qt_module = framework_->GetModuleManager()->GetModule<QtUI::QtModule>
        (Foundation::Module::MT_Gui).lock();

    // If this occurs, we're most probably operating in headless mode.
    if (qt_module.get() == 0)
        return;

    canvas_ = qt_module->CreateCanvas(QtUI::UICanvas::External).lock();
    
    /** \todo Just instantiating a QUiLoader on the next code line below causes 
          11 memory leaks to show up, like the following:
            {106636} normal block at 0x09D6B1F0, 68 bytes long.
            {106635} normal block at 0x09D6C770, 16 bytes long.
            {104970} normal block at 0x09D689E8, 68 bytes long.
            {104969} normal block at 0x09D6B728, 16 bytes long.
            {104910} normal block at 0x09D6B5A8, 4 bytes long.
            {2902} normal block at 0x018CE528, 24 bytes long.
            {2901} normal block at 0x018CE480, 104 bytes long.
            {2900} normal block at 0x018CDD10, 8 bytes long.
            {2899} normal block at 0x018CE428, 24 bytes long.
            {2898} normal block at 0x018CE3D0, 24 bytes long.
            {2897} normal block at 0x018CE358, 56 bytes long.

            Figure something out.. */

    QUiLoader loader;
    QFile file("./data/ui/login.ui");

    if (!file.exists())
    {
        RexLogicModule::LogError ("cannot find login window .ui file.");
        return;
    }

    login_widget_ = loader.load(&file); 
    file.close();

    ///\todo Here we have strange and not-so-wanted feature.
    /// If you first addWidget (in Internal-canvas) and then SetCanvasSize() result:
    /// only partial window is seen. Must investigate futher that why this happends.

    // Set canvas size. 
    QSize size = login_widget_->size();
    canvas_->SetCanvasSize(size.width(), size.height());

    canvas_->AddWidget(login_widget_);

    // Set canvas scrollbar policy
    canvas_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    canvas_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Create connections.
    QPushButton *pButton = login_widget_->findChild<QPushButton *>("but_connect");
    QObject::connect(pButton, SIGNAL(clicked()), this, SLOT(Connect()));

    pButton = login_widget_->findChild<QPushButton *>("but_log_out");
    QObject::connect(pButton, SIGNAL(clicked()), this, SLOT(Disconnect()));

    pButton = login_widget_->findChild<QPushButton *>("but_quit");
    QObject::connect(pButton, SIGNAL(clicked()), this, SLOT(Quit()));

    // Recover the connection settings that were used in previous login
    // from the xml configuration file.
    std::string strText = "";
    std::string strGroup = "Login";
    std::string strKey = "username";

    strText = framework_->GetDefaultConfigPtr()->GetSetting<std::string>(strGroup, strKey);

    // Opensim username:
    QLineEdit *line = login_widget_->findChild<QLineEdit* >("line_user_name");
    line->setText(QString(strText.c_str()));

    strKey = "server";
    strText = framework_->GetDefaultConfigPtr()->GetSetting<std::string>(strGroup, strKey);

    // Opensim server: 
    line = login_widget_->findChild<QLineEdit* >("line_server");
    line->setText(QString(strText.c_str()));

    // Rex auth:
    line = login_widget_->findChild<QLineEdit* >("line_server_au");
    line->setText(QString(strText.c_str()));

    strKey = "auth_name";
    strText = framework_->GetDefaultConfigPtr()->GetSetting<std::string>(strGroup, strKey);

    // Rex auth: 
    line = login_widget_->findChild<QLineEdit* >("line_login_au");
    line->setText(QString(strText.c_str()));

    strKey = "auth_server";
    strText = framework_->GetDefaultConfigPtr()->GetSetting<std::string>(strGroup, strKey);

    // Rex auth: 
    line = login_widget_->findChild<QLineEdit* >("line_auth_server");
    line->setText(QString(strText.c_str()));

    canvas_->Show();

    CreateLogoutMenu();
}

void RexLoginWindow::CreateLogoutMenu()
{
    Foundation::ModuleSharedPtr qt_module = framework_->GetModuleManager()->GetModule("QtModule").lock();
    QtUI::QtModule *qt_ui = dynamic_cast<QtUI::QtModule*>(qt_module.get());

    // If this occurs, we're most probably operating in headless mode.
    if (!qt_ui)
        return;

    screen_canvas_ = qt_ui->CreateCanvas(QtUI::UICanvas::Internal).lock();
    screen_canvas_->SetCanvasSize(128, 128);
    screen_canvas_->SetCanvasResizeLock(true);

    QSize size = screen_canvas_->GetRenderWindowSize();
    screen_canvas_->SetPosition(size.width()-128, size.height()-150);

    logout_button_ = new QPushButton();
    logout_button_->setText("Log out");
    logout_button_->move(5, 5);
    QObject::connect(logout_button_, SIGNAL(clicked()), this, SLOT(DisconnectAndShowLoginWindow()));
    screen_canvas_->AddWidget(logout_button_);

    quit_button_ = new QPushButton();
    quit_button_->setText("Quit");
    quit_button_->move(5, 30);
    QObject::connect(quit_button_, SIGNAL(clicked()), this, SLOT(Quit()));
    screen_canvas_->AddWidget(quit_button_);

    screen_canvas_->SetLockPosition(true);
    screen_canvas_->Hide();
}

void RexLoginWindow::Connect()
{
    // Check which tab is active.
    QTabWidget *widget = login_widget_->findChild<QTabWidget* >("tabWidget");
    int index = widget->currentIndex();

    bool successful = false;
    std::string user_name = "";
    std::string server_address = "";

    switch(index)
    {
    case 1:
    {
        // Open sim
        QLineEdit *line = login_widget_->findChild<QLineEdit* >("line_user_name");
        user_name = line->text().toStdString();
        line =  login_widget_->findChild<QLineEdit* >("line_server");
        server_address = line->text().toStdString();

        // password
        line = login_widget_->findChild<QLineEdit* >("line_password");
        std::string password = line->text().toStdString();

        successful = rex_logic_->GetServerConnection()->ConnectToServer(user_name, password, server_address);
        if (successful)
        {
            // Save login and server settings for future use. 
            framework_->GetConfigManager()->SetSetting<std::string>(std::string("Login"), std::string("server"), server_address);
            framework_->GetConfigManager()->SetSetting<std::string>(std::string("Login"), std::string("username"), user_name);
        }

        break;
    }
    case 2:
    {
        // Rex authentication 
        QLineEdit* line = 0;
        user_name ="";
        line =  login_widget_->findChild<QLineEdit* >("line_server_au");
        server_address = line->text().toStdString();

        // password
        line = login_widget_->findChild<QLineEdit* >("line_password_au");
        std::string password = line->text().toStdString();

        line = login_widget_->findChild<QLineEdit* >("line_login_au");
        std::string auth_login = line->text().toStdString();

        // START HACK because some reason authentication server needs a last name when system logs into
        // localhost authentication server so we need to do this. 
        user_name = auth_login + " " + auth_login;
        // END 

        line = login_widget_->findChild<QLineEdit* >("line_auth_server");
        std::string auth_server = line->text().toStdString();

        successful = rex_logic_->GetServerConnection()->ConnectToServer(user_name, password, server_address, auth_server, auth_login);
        // Because hack we clear this, just in case.
        user_name="";

        if (successful)
        {
            // Save login and server settings for future use. 
            framework_->GetConfigManager()->SetSetting<std::string>(std::string("Login"), std::string("server"), server_address);

            // Save login and server settings for future use. 
            framework_->GetConfigManager()->SetSetting<std::string>(std::string("Login"), std::string("auth_server"), auth_server);
            framework_->GetConfigManager()->SetSetting<std::string>(std::string("Login"), std::string("auth_name"), auth_login);
        }

        break;
    }
    case 0:
    {
        // OpenID
        /* 
            // OLD OPENID LOGIN
            if(cblogin == 0)
                cblogin = new CBLoginWidget();
            cblogin->show();
            QObject::connect(cblogin, SIGNAL( loginProcessed(QString) ), SLOT( processCBLogin(QString) ));
            successful = false;
            break;
        */

        QComboBox *comboBoxAddress = login_widget_->findChild<QComboBox* >("comboBox_Address");
        webLogin = new RexWebLogin(0, comboBoxAddress->currentText());
        webLogin->show();
        QObject::connect(webLogin, SIGNAL( LoginProcessed(QString) ), SLOT( ProcessCBLogin(QString) ));
        successful = false;

        break;
    }
    default:
        break;
    }
}

void RexLoginWindow::DisconnectAndShowLoginWindow()
{
    Disconnect();
    ShowLoginWindow();
}

void RexLoginWindow::HideLoginWindow()
{
    canvas_->Hide();
    //login_widget_->hide();
    screen_canvas_->Show();
}

void RexLoginWindow::ShowLoginWindow()
{
    canvas_->Show();
    screen_canvas_->Hide();
}

void RexLoginWindow::Disconnect()
{
    // Disconnect from server
    rex_logic_->LogoutAndDeleteWorld();
}

void RexLoginWindow::Quit()
{
    if (rex_logic_->GetServerConnection()->IsConnected())
        rex_logic_->LogoutAndDeleteWorld();

    framework_->Exit();
}

void RexLoginWindow::ProcessCBLogin(QString inresult)
{
    size_t pos1, pos2;
    std::string result = inresult.toStdString();

    pos1 = result.find("http://");
    pos2 = result.find("?");
    std::string address = result.substr(pos1, pos2);
    pos1 = result.find("&", pos2);
    std::string firstname = result.substr(pos2+1, pos1-pos2-1);
    pos2 = result.find("&", pos1+1);
    std::string lastname = result.substr(pos1+1, pos2-pos1-1);
    pos1 = result.find("&", pos2+1);
    std::string identityUrl = result.substr(pos2+1, result.length());

    Poco::URI uri = Poco::URI(address);
    int port = uri.getPort();

    rex_logic_->GetServerConnection()->ConnectToCableBeachServer(firstname, lastname, port, address);
}

void RexLoginWindow::UpdateConnectionStateToUI(OpenSimProtocol::Connection::State state)
{
    ///\todo Perhaps this state change should come from above instead of this function.
    if (state == OpenSimProtocol::Connection::STATE_CONNECTED)
        HideLoginWindow();
    else
        ShowLoginWindow();

    if (login_widget_ != 0)
    {
        QLabel *pLabel = login_widget_->findChild<QLabel* >("lab_state");
        pLabel->setText(QString(OpenSimProtocol::Connection::NetworkStateToString(state).c_str()));
    }
}

}

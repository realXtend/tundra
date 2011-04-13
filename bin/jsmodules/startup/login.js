
engine.ImportExtension("qt.core");
engine.ImportExtension("qt.gui");

var widget;
var server_address;
var user_name;
var user_password;
var login_button;

// Only show login window if we are using a viewer.
if (!server.IsAboutToStart())
    SetupLoginScreen();

function SetupLoginScreen()
{
    widget = new QWidget();
    widget.setLayout(new QVBoxLayout());

    widget.setWindowState(Qt.WindowFullScreen);
    var child = uiservice.LoadFromFile("./data/ui/LoginWidget.ui", false);
    child.setParent(widget);
    uiservice.AddWidgetToScene(widget);
    widget.setVisible(true);

    widget.layout().addWidget(child, 0, 0);
    widget.layout().setContentsMargins(0, 0, 0, 0);

    // Background image
    var loginScreenWidget = findChild(widget, "mainFrame");
    loginScreenWidget.setStyleSheet("QFrame#mainFrame { border-image: url('./data/ui/images/login_background.png') } ");

    login_button = findChild(widget, "loginButton");
    login_button.clicked.connect(LoginPressed);
    server_address = findChild(widget, "serverAddressLineEdit");

    // Sandbox logo
    var logoLabel = findChild(widget, "logoLabel");
    logoLabel.pixmap = new QPixmap("./data/ui/images/sandbox_logo.png");

    client.Connected.connect(HideLoginScreen);
    client.Disconnected.connect(ShowLoginScreen);
}

function TrimField(txt)
{
    return txt.replace(/^\s+|\s+$/g,"");
}

function LoginPressed()
{
    client.ClearLoginProperties();
    //client.SetLoginProperty("Username", TrimField(user_name.text));
    //client.SetLoginProperty("Password", TrimField(user_password.text));
    client.Login(TrimField(server_address.text), 2345);
}

function HideLoginScreen()
{
    widget.setVisible(false);
}

function ShowLoginScreen()
{
    widget.setVisible(true);
}

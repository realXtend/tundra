// !ref: local://LoginWidget.ui

engine.ImportExtension("qt.core");
engine.ImportExtension("qt.gui");

var widget;
var serverAddressLineEdit;
var usernameLineEdit;
var passwordLineEdit;
var loginButton;
var exitButton;
var tcpButton;
var udpButton;

// Only show login window if we are using a viewer.
if (!server.IsAboutToStart())
    SetupLoginScreen();

function SetupLoginScreen() {
    widget = new QWidget();
    widget.setLayout(new QVBoxLayout());

    widget.setWindowState(Qt.WindowFullScreen);
    var child = ui.LoadFromFile("./data/ui/LoginWidget.ui", false);
    child.setParent(widget);
    ui.AddWidgetToScene(widget);
    widget.setVisible(true);

    widget.layout().addWidget(child, 0, 0);

    loginButton = findChild(widget, "pushButton_Connect");
    loginButton.clicked.connect(LoginPressed);

    exitButton = findChild(widget, "pushButton_Exit");
    exitButton.clicked.connect(Exit);

    serverAddressLineEdit = findChild(widget, "lineEdit_WorldAddress");
    usernameLineEdit = findChild(widget, "lineEdit_Username");
    passwordLineEdit = findChild(widget, "lineEdit_Password");
    tcpButton = findChild(widget, "radioButton_ProtocolTCP");
    udpButton = findChild(widget, "radioButton_ProtocolUDP");

    var logoLabel = findChild(widget, "label_ClientLogo");
    logoLabel.pixmap = new QPixmap("./data/ui/images/realxtend_logo.png");

    client.Connected.connect(HideLoginScreen);
    client.Disconnected.connect(ShowLoginScreen);
}

function TrimField(txt) {
    return txt.replace(/^\s+|\s+$/g,"");
}

function LoginPressed() {
    client.ClearLoginProperties();

    var username = TrimField(usernameLineEdit.text);
    var password = TrimField(passwordLineEdit.text);
    var protocol = "";
    if (tcpButton.checked)
        protocol = "tcp";
    else if (udpButton.checked)
        protocol = "udp";

    var port = 2345;
    var strings = TrimField(serverAddressLineEdit.text).split(':');
    if (strings.length > 1)
        port = parseInt(strings[1]);

    client.Login(strings[0], port, username, password, protocol);
}

function HideLoginScreen() {
    widget.setVisible(false);
}

function ShowLoginScreen() {
    widget.setVisible(true);
}

function Exit() {
    framework.Exit();
}
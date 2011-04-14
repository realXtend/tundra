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

var configServer = null;
var configUsername = null;
var configProtocol = null;

function SetupLoginScreen() {
    widget = new QWidget();
    widget.setLayout(new QVBoxLayout());
	widget.layout().setContentsMargins(0,0,0,0);

    var child = ui.LoadFromFile("./data/ui/LoginWidget.ui", false);
    child.setParent(widget);
    widget.setVisible(false);
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
    client.Connected.connect(WriteConfigFromUi);
    client.Disconnected.connect(ShowLoginScreen);

    ReadConfigToUi();
    return widget;
}

function ReadConfigToUi() {
    // Make double sure with "" default value and null 
    // checks that we can in any case insert var to ui
    configServer = framework.Config().Get(configFile, configSection, "login_server", "");
    if (configServer == null)
        configServer = "";
    configUsername = framework.Config().Get(configFile, configSection, "login_username", "");
    if (configUsername == null)
        configUsername = "";
    configProtocol = framework.Config().Get(configFile, configSection, "login_protocol", "");
    if (configProtocol == null)
        configProtocol = "";

    serverAddressLineEdit.text = configServer;
    usernameLineEdit.text = configUsername;
    if (configProtocol == "tcp")
        tcpButton.checked = true;
    else if (configProtocol == "udp")
        udpButton.checked = true;
}

function WriteConfigFromUi() {
    // Downside of this is that user may do something to the UI elements while loggin in.
    // In Tundra its so fast that doubt that will happen. Can be fixed to read in to configX values in LoginPresse()
    framework.Config().Set(configFile, configSection, "login_server", TrimField(serverAddressLineEdit.text));
    framework.Config().Set(configFile, configSection, "login_username", TrimField(usernameLineEdit.text));
    framework.Config().Set(configFile, configSection, "login_protocol", GetProtocol());
}

function TrimField(txt) {
    return txt.replace(/^\s+|\s+$/g,"");
}

function GetProtocol() {
    if (tcpButton.checked)
        return "tcp";
    else if (udpButton.checked)
        return "udp";
    return "";
}

function LoginPressed() {
    client.ClearLoginProperties();

    var username = TrimField(usernameLineEdit.text);
    var password = TrimField(passwordLineEdit.text);
    var protocol = GetProtocol();
    if (protocol == "")
        return;
    
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
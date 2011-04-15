// !ref: local://LoginWebWidget.ui
// !ref: local://LoginWebSettings.ui

engine.ImportExtension("qt.core");
engine.ImportExtension("qt.gui");
engine.ImportExtension("qt.webkit");

engine.IncludeFile("jsmodules/lib/class.js");
engine.IncludeFile("jsmodules/startup/LoginScreen.js");

var iconRefresh = new QIcon("./data/ui/images/browser/refresh.png");
var iconStop = new QIcon("./data/ui/images/browser/stop.png");

// This magic number should do the trick of keeping the top part
// of the UI same if you are in 3D tab or web tab. Hopefully the linux
// and other operating systems wont get into trouble here (like different font height etc).
// There are minimum sizes for all widgets
var magicHeightValue = 81;

// Browser manager controls the web login widget.
// This object will use private p_ variable in 
// functions that are potentially called from outside.
var BrowserManager = Class.extend
({
    init: function()
    {
        this.connected = false;
        this.squeezeEnabled = false;
        
        this.settings = new BrowserSettings(this);

        var uiBase = "./data/ui/";
        var imageBase = uiBase + "images/";

        // Load widget and init children etc.
        this.browser = ui.LoadFromFile(uiBase + "LoginWebWidget.ui", false);
        this.browser.setWindowState(Qt.WindowFullScreen);
        this.browser.windowFlags = Qt.Widget;
        
        this.browserProxy = ui.AddWidgetToScene(this.browser, Qt.Widget);
        this.browserProxy.windowFlags = Qt.Widget;
        this.browserProxy.effect = 0;
        
        this.tabs = findChild(this.browser, "tabs");
        this.tabs.clear();
        
        this.mainFrame = findChild(this.browser, "mainFrame");
        if (ui.MainWindow() && ui.MainWindow().menuBar())
            this.mainFrame.layout().setContentsMargins(0,ui.MainWindow().menuBar().height-5,0,0)

        // Load the login screen
        this.loginscreen = SetupLoginScreen();
        this.tabs.addTab(this.loginscreen, "Tundra");
        
        // Kind of a hack on how to put this in its place
        var newTabButton = new QPushButton(this.browser);
        newTabButton.tooltip = "Open New Tab";
        newTabButton.flat = true;
        newTabButton.setFixedSize(16,16);
        newTabButton.setStyleSheet("background-image: url(./data/ui/images/browser/tab-new.png); border: 0px;");
        newTabButton.move(3,60); // Here be dragons

        // Address and progress bar
        this.addressBar = new QComboBox();
        this.addressBar.setFixedHeight(23);
        this.addressBar.editable = true;
        this.progressBar = findChild(this.browser, "progressBar");
        this.progressBar.visible = false;
        
        var controlLayout = findChild(this.browser, "controlLayout");
        
        // Browser controls
        this.browserToolBar = new QToolBar();
        this.browserToolBar.setFixedHeight(26);
        this.browserToolBar.setStyleSheet("background-color: none; border: 0px;");
        this.browserToolBar.toolButtonStyle = Qt.ToolButtonIconOnly;
        this.browserToolBar.orientation = Qt.Horizontal;
        this.browserToolBar.iconSize = new QSize(23,23);
        this.browserToolBar.floatable = false;
        this.browserToolBar.movable = false;
        
        this.actionBack = this.browserToolBar.addAction(new QIcon(imageBase + "browser/back.png"), "");
        this.actionBack.triggered.connect(this.onBack);
        this.actionBack.toolTip = "Back";
        this.actionForward = this.browserToolBar.addAction(new QIcon(imageBase + "browser/forward.png"), "");
        this.actionForward.triggered.connect(this.onForward);
        this.actionForward.toolTip = "Forward";
        this.actionRefreshStop = this.browserToolBar.addAction(iconRefresh, "");
        this.actionRefreshStop.triggered.connect(this.onRefreshStop);
        this.actionRefreshStop.tooltip = "Refresh";
        this.actionHome = this.browserToolBar.addAction(new QIcon(imageBase + "browser/home.png"), "");
        this.actionHome.triggered.connect(this.onHome);
        this.actionHome.toolTip = "Go to home page " + this.homepage;
        
        // Toolbar for inworld widgets
        // \todo change to QToolBar
        this.toolBar = new QWidget();
        this.toolBar.setFixedHeight(26);
        this.toolBar.setLayout(new QHBoxLayout());
        this.toolBar.layout().setContentsMargins(0,0,0,0);
        
        // Toolbar for address bar and favorites
        this.favoritesToolBar = new QToolBar();
        this.favoritesToolBar.setFixedHeight(26);
        this.favoritesToolBar.setStyleSheet("background-color: none; border: 0px;");
        this.favoritesToolBar.toolButtonStyle = Qt.ToolButtonIconOnly;
        this.favoritesToolBar.orientation = Qt.Horizontal;
        this.favoritesToolBar.iconSize = new QSize(23,23);
        this.favoritesToolBar.floatable = false;
        this.favoritesToolBar.movable = false;
        this.actionAddFavorite = this.favoritesToolBar.addAction(new QIcon(imageBase + "browser/favorites.png"), "");
        this.actionAddFavorite.triggered.connect(this.onFavoritePressed);
        this.actionAddFavorite.tooltip = "Add as home page or add to bookmarks";
        this.actionSettings = this.favoritesToolBar.addAction(new QIcon(imageBase + "browser/settings.png"), "");
        this.actionSettings.triggered.connect(this.settings.onSettingsPressed);
        this.actionSettings.tooltip = "Browser settings";
        
        this.addressAndFavoritesBar = new QWidget();
        this.addressAndFavoritesBar.setFixedHeight(26);
        this.addressAndFavoritesBar.setLayout(new QHBoxLayout());
        this.addressAndFavoritesBar.layout().setContentsMargins(0,0,0,0);
        this.addressAndFavoritesBar.layout().addWidget(this.addressBar, 1, 0);
        this.addressAndFavoritesBar.layout().addWidget(this.favoritesToolBar, 0, 0);

        // Splitter
        this.splitter = new QSplitter(Qt.Horizontal);
        this.splitter.addWidget(this.addressAndFavoritesBar);
        this.splitter.addWidget(this.toolBar);
        this.splitter.setFixedHeight(26);
        this.splitter.handleWidth = 12;
        this.splitter.setStretchFactor(0, 2);
        
        // Combine ui
        controlLayout.addWidget(this.browserToolBar, 0, 0);
        controlLayout.addWidget(this.splitter, 0, 0);
        
        // Connect signals
        this.addressBar.lineEdit().returnPressed.connect(this.onAddressBarRequest);
        this.tabs.tabCloseRequested.connect(this.onTabCloseRequest);
        this.tabs['currentChanged(int)'].connect(this.onTabIndexChanged);
        
        newTabButton.clicked.connect(this.onTabNewRequest);
        
        client.Connected.connect(this.onConnected);
        client.Disconnected.connect(this.onDisconnected);
    },
    
    start: function()
    {
        this.setVisible(true);
        this.onTabIndexChanged(this.tabs.currentIndex);
        
        if (this.settings.startupLoadHomePage)
        {
            if (HasTundraScheme(this.settings.homepage))
                if (!this.settings.startupConnectToHomePage)
                    return;
            this.openUrl(this.settings.homepage);
        }
    },
    
    setVisible: function(visible)
    {
        this.browser.visible = visible;
    },

    getCurrentWidget: function()
    {
        return p_.tabs.widget(p_.tabs.currentIndex);
    },
    
    addTool: function(widget, index)
    {
        if (index == null)
            index = -1;
        p_.toolBar.layout().insertWidget(index, widget, 0, 0);
        widget.destroyed.connect(p_.refreshSplitter);
    },
    
    refreshSplitter: function(obj)
    {
        p_.splitter.setStretchFactor(0, 2);
        p_.splitter.refresh();
    },
    
    refreshSqueezer: function()
    {
        if (this.connected && this.tabs.currentIndex == 0)
        {
            this.browser.maximumHeight = magicHeightValue; // Here be dragons
            this.squeezeEnabled = true;
            this.loginscreen.visible = false;
        }
        else
        {
            this.browser.maximumHeight = 10000;
            this.browser.size = ui.MainWindow().size;
            this.squeezeEnabled = false;
        }
    },
    
    openUrl: function(url)
    {
        // Check if current is classic login/3d tab
        tab = new BrowserTab(p_.tabs, p_.tabCallBack);
        tab.load(url);
    },

    onConnected: function()
    {
        p_.connected = true;
        p_.refreshSqueezer();
        p_.onTabIndexChanged(p_.tabs.currentIndex);
    },
    
    onDisconnected: function()
    {
        p_.connected = false;
        p_.refreshSqueezer();
        p_.onTabIndexChanged(p_.tabs.currentIndex);
        if (p_.tabs.currentIndex != 0)
        {
            p_.tabs.setTabToolTip(0, "Login");
            p_.tabs.setTabText(0, "Login")
        }
    },
    
    onFavoritePressed: function()
    {
        var addressBarInput = p_.addressBar.lineEdit().text;
        var tabsCurrentIndex = p_.tabs.currentIndex;
        
        var messageBox = new QMessageBox(QMessageBox.NoIcon, 
                                         "Add To Favorites", 
                                         addressBarInput, 
                                         QMessageBox.NoButton,
                                         ui.MainWindow(),
                                         Qt.Tool);
        
        // Disable bookmarks for now
        // \todo Implement bookmarks
        messageBox.addButton("Set As Homepage", QMessageBox.YesRole);
        messageBox.addButton("Add To Bookmarks", QMessageBox.AcceptRole).enabled = false;
        messageBox.addButton("Cancel", QMessageBox.NoRole);
        messageBox.iconPixmap = new QPixmap("./data/ui/images/browser/favorites.png");
        
        var result = messageBox.exec();
        // Return is StandarButton not ButtonRole
        if (result == 0) // QMessageBox.YesRole
        {
            p_.settings.homepage = addressBarInput;
            p_.settings.writeConfig();
        }
        else if (result == 1) // QMessageBox.AcceptRole
        {
            // \todo implement bookmark saving
        }
    },
    
    onAddressBarRequest: function()
    {
        var addressBarInput = p_.addressBar.lineEdit().text;
        var tabsCurrentIndex = p_.tabs.currentIndex;
        if (tabsCurrentIndex == 0)
        {
            if (HasTundraScheme(addressBarInput))
                p_.tabCallBack("weblogin", addressBarInput, tabsCurrentIndex);
            return;
        }
        
        if (!HasTundraScheme(addressBarInput))
        {
            var url = QUrl.fromUserInput(addressBarInput);
            p_.getCurrentWidget().load(url);
        }
        else
        {
            p_.tabCallBack("weblogin", addressBarInput, tabsCurrentIndex);
        }
    },
    
    onTabNewRequest: function()
    {
        if (HasTundraScheme(p_.settings.homepage))
        {
            // This should never happen, settings will prevent you from
            // inserting a tundra:// url as new tab url. But user might
            // set it manually to config so lets still handle the case.
            if (HasTundraScheme(p_.settings.newTabUrl))
            {
                p_.tabCallBack("weblogin", p_.settings.newTabUrl, 0);
                p_.tabs.currentIndex = 0;
            }
            else
                p_.openUrl(p_.settings.newTabUrl);
            return;
        }
            
        if (p_.settings.newTabOpenHomepage)
            p_.openUrl(p_.settings.homepage);
        else
            p_.openUrl(p_.settings.newTabUrl);
    },
    
    onTabIndexChanged: function(index)
    {
        if (index != 0)
        {
            var tab = p_.tabs.widget(index);
            p_.addressBar.lineEdit().text = tab.url.toString();
            p_.actionAddFavorite.enabled = true;
            p_.progressBar.visible = false;
            p_.actionBack.enabled = true;
            p_.actionForward.enabled = true;
            p_.actionRefreshStop.enabled = true;
        }
        else
        {
            p_.progressBar.visible = false;
            p_.actionBack.enabled = false;
            p_.actionForward.enabled = false;
            p_.actionRefreshStop.enabled = false;
            
            if (p_.connected)
            {
                // Login must not end in "/" or it wont look proper
                var loginPropAddress = client.GetLoginProperty("address");
                if (loginPropAddress.charAt(loginPropAddress.length-1) == "/")
                    loginPropAddress = loginPropAddress.substring(0, loginPropAddress.length-1);
                
                // Add all the relevant params we can find from the login properties, skip password though
                var tundraUrl = "tundra://" + loginPropAddress + ":" + client.GetLoginProperty("port");
                tundraUrl = tundraUrl + "/?username=" + client.GetLoginProperty("username");
                if (client.GetLoginProperty("protocol") != "")
                    tundraUrl = tundraUrl + "&protocol=" + client.GetLoginProperty("protocol");
                if (client.GetLoginProperty("avatarurl") != "")
                    tundraUrl = tundraUrl + "&avatarurl=" + client.GetLoginProperty("avatarurl");

                p_.addressBar.lineEdit().text = tundraUrl;
                p_.actionAddFavorite.enabled = true;
                
                p_.tabs.setTabToolTip(0, tundraUrl);
                var tundraUrlShortened = tundraUrl.substring(9);
                if (tundraUrlShortened.length > 23)
                    tundraUrlShortened = tundraUrlShortened.substring(0,20) + "...";
                p_.tabs.setTabText(0, tundraUrlShortened);
            }
            else
            {
                p_.addressBar.lineEdit().text = "local://LoginWidget.ui";
                p_.actionAddFavorite.enabled = false;
                p_.tabs.setTabToolTip(0, "Login");
                p_.tabs.setTabText(0, "Login")
            }
        }
        p_.refreshSqueezer();
    },
    
    onTabCloseRequest: function(index)
    {
        if (index == 0)
            return;
        p_.tabs.widget(index).stop();
        p_.tabs.widget(index).close();
        p_.tabs.removeTab(index);
    },
    
    onBack: function()
    {
        p_.getCurrentWidget().back();
    },
    
    onForward: function()
    {
        p_.getCurrentWidget().forward();
    },
    
    onHome: function()
    {
        if (HasTundraScheme(p_.settings.homepage))
        {
            p_.tabCallBack("weblogin", p_.settings.homepage, p_.tabs.currentIndex);
            return;
        }
        
        var qUrl = QUrl.fromUserInput(p_.settings.homepage);
        if (p_.tabs.currentIndex == 0)
        {
            p_.openUrl(qUrl);
        }
        else
        {
            p_.getCurrentWidget().load(qUrl);
        }
    },
    
    onRefreshStop: function()
    {
        var currentTooltip = p_.actionRefreshStop.tooltip;
        if (currentTooltip == "Refresh")
            p_.getCurrentWidget().reload();
        else if (currentTooltip == "Stop")
            p_.getCurrentWidget().stop();
    },

    tabCallBack: function(event, param, index)
    {
        if (event == "started")
        {
            // Never show load bar if squeeze is enabled
            if (!p_.squeezeEnabled) 
                p_.progressBar.visible = true;
            p_.progressBar.value = 0;
            p_.actionRefreshStop.icon = iconStop;
            p_.actionRefreshStop.tooltip = "Stop";
        }
        else if (event == "finished")
        {
            p_.progressBar.visible = false;
            p_.actionRefreshStop.icon = iconRefresh;
            p_.actionRefreshStop.tooltip = "Refresh";
            if (param != "" && p_.tabs.currentIndex == index)
                p_.addressBar.lineEdit().text = param;
        }
        else if (event == "failed")
        {
            p_.progressBar.visible = false;
            p_.actionRefreshStop.icon = iconRefresh;
            p_.actionRefreshStop.tooltip = "Refresh";
        }
        else if (event == "progress")
        {
            // Hide if we are in the 3D/tundra login tab
            if (p_.tabs.currentIndex == 0)
            {
                if (p_.progressBar.visible)
                    p_.progressBar.visible = false;
                return;
            }
            
            // Show and update if we are in the same tab, otherwise hide
            if (p_.tabs.currentIndex == index)
            {
                if (!p_.progressBar.visible)
                    p_.progressBar.visible = true;
                p_.progressBar.value = param;
            }
            else
            {
                if (p_.progressBar.visible)
                    p_.progressBar.visible = false;
            }
        }
        else if (event == "weblogin")
        {
            p_.actionRefreshStop.icon = iconRefresh;
            p_.actionRefreshStop.tooltip = "Refresh";
            
            // We could directly call client.Login(new QUrl(param));
            // but we want to some validatons so we can give and error for the user right here
            // as the login function will just return.
            var loginInfo = ParseTundraLoginInfo(param);
            if (loginInfo["username"] != null && loginInfo["address"] != null)
            {
                if (loginInfo["username"] != "" && loginInfo["address"] != "")
                {
                    // Disconnect if connected
                    if (p_.connected)
                        client.Logout();
                    // Focus 3D tab and close invoking tab
                    if (index != 0)
                    {
                        p_.tabs.currentIndex = 0;
                        p_.tabs.removeTab(index);
                    }
                    p_.loginscreen.visible = false;
                    // Perform login
                    var qLoginUrl = new QUrl.fromUserInput(param);
                    client.Login(qLoginUrl);
                }
            }
        }
    },
});

var BrowserTab = Class.extend
({
    init: function(tabs, callback)
    {
        this.webview = new QWebView();
        this.webview.page().forwardUnsupportedContent = true;
        
        this.callback = callback;
        
        this.tabs = tabs;
        this.tabs.addTab(this.webview, "");
        
        this.webview.loadStarted.connect(this, this.loadStarted);
        this.webview.loadFinished.connect(this, this.loadFinished);
        this.webview.loadProgress.connect(this, this.loadProgress);
        this.webview.page().unsupportedContent.connect(this, this.unsupportedContent);
        
        tabs.currentIndex = this.myIndex();
    },

    myIndex: function()
    {
        return this.tabs.indexOf(this.webview);
    },

    myUrl: function()
    {
        return this.webview.url.toString();
    },

    load: function(url)
    {
        this.webview.load(new QUrl(url))
    },
    
    updateName: function(name)
    {
        this.tabs.setTabToolTip(this.myIndex(), name);
        var name = this.formName(name);
        this.tabs.setTabText(this.myIndex(), name);
    },
    
    formName: function(name)
    {
        if (name.substring(0,7) == "http://")
            name = name.substring(7);
        else if (name.substring(0,8) == "https://")
            name = name.substring(8);
        if (name.charAt(name.length-1) == "/")
            name = name.substring(0,name.length-1);
        if (name.length > 23)
            name = name.substring(0, 20) + "...";
        return name;
    },
    
    loadStarted: function()
    {
        this.updateName("Loading...");
        this.callback("started");
    },
    
    loadProgress: function(step)
    {
        this.callback("progress", step, this.myIndex());
    },
    
    unsupportedContent: function(reply)
    {
        var replyUrl = reply.url().toString();
        print("unsupportedContent:" + replyUrl);
        if (HasTundraScheme(replyUrl))
            this.callback("weblogin", replyUrl, this.myIndex());
    },
    
    loadFinished: function(success)
    {
        var url = this.myUrl();
        if (success)
        {
            if (url == "about:blank")
            {
                this.updateName("Page Load Error");
                this.callback("finished", "", this.myIndex());
            }
            else
            {
                this.updateName(url);
                this.callback("finished", url, this.myIndex());
            }
        }
        else
        {
            var requestedUrl = this.webview.page().mainFrame().requestedUrl.toString();
            if (HasTundraScheme(requestedUrl))
            {
                this.callback("weblogin", requestedUrl, this.myIndex());
                return;
            }
            
            this.updateName("Page Load Error");
            this.callback("failed");

            // webview.url() is not set to the failed address, we need to change the way
            // BrowserManager call us onAddressBarRequest to call tab.load not webview.load
            //var html = "<p>The page " + url + " could not be loaded...</p>";
            var html = "<p>The page could not be loaded...</p>";
            this.webview.page().mainFrame().setHtml(html);
        }
    }
});

var BrowserSettings = Class.extend
({
    init: function(browserManager)
    {
        this.browserManager = browserManager;
        
        this.configFile = "browsersettings";
        this.urlSection = "url";
        this.behaviourSection = "behaviour";
        
        this.widget = ui.LoadFromFile("./data/ui/LoginWebSettings.ui", false);
        this.widget.setWindowFlags(Qt.Tool);
        this.widget.visible = false;
        
        var button = null;
        button = findChild(this.widget, "buttonSave");
        button.clicked.connect(this.onSettingsSave);
        button = findChild(this.widget, "buttonCancel");
        button.clicked.connect(this.onSettingsCancel);
        
        this.readConfig();
    },
    
    onSettingsPressed: function()
    {
        var p_s = p_.settings;
        if (p_s.widget.visible)
        {
            p_s.widget.visible = false;
            return;
        }
        p_s.setCurrentToUi();
        
        var mainWinPosi = ui.MainWindow().pos;
        var mainWinSize = ui.MainWindow().size;
        
        var center_x = mainWinPosi.x() + (mainWinSize.width() / 2);
        var center_y = mainWinPosi.y() + (mainWinSize.height() / 2);       
        
        p_s.widget.pos = new QPoint(center_x - (p_s.widget.width / 2), center_y - (p_s.widget.height / 2));
        p_s.widget.visible = true;
    },
    
    onSettingsSave: function()
    {
        var p_s = p_.settings;
        var child = null;
        
        child = findChild(p_s.widget, "newTabPageLineEdit");
        if (child.text.toLowerCase().substring(0,9) == "tundra://")
        {
            var errorBox = new QMessageBox(QMessageBox.Warning, "Invalid new tab page",
                                             "The new tab url cannot be a tundra:// server", QMessageBox.NoButton,
                                             p_s.widget, Qt.Tool);
            errorBox.addButton("Close", QMessageBox.NoRole);
            errorBox.exec();
            return;
        }
        p_s.newTabUrl = QUrl.fromUserInput(child.text).toString();
        
        child = findChild(p_s.widget, "homePageLineEdit");
        p_s.homepage = QUrl.fromUserInput(child.text).toString();
               
        child = findChild(p_s.widget, "openHomePageOnNewTab");
        p_s.newTabOpenHomepage = child.checked;
        
        child = findChild(p_s.widget, "loadHomePageOnStartup");
        p_s.startupLoadHomePage = child.checked;
        
        child = findChild(p_s.widget, "connectToHomePage");
        p_s.startupConnectToHomePage = child.checked;
        
        p_.settings.widget.visible = false;
        p_s.writeConfig();
    },
    
    onSettingsCancel: function()
    {
        p_.settings.widget.visible = false;
    },
    
    setCurrentToUi: function()
    {
        var p_s = p_.settings;
        var child = null;
        
        child = findChild(p_s.widget, "homePageLineEdit");
        child.text = p_s.homepage;

        child = findChild(p_s.widget, "newTabPageLineEdit");
        child.text = p_s.newTabUrl;
        
        child = findChild(p_s.widget, "openHomePageOnNewTab");
        child.checked = p_s.newTabOpenHomepage;
        
        child = findChild(p_s.widget, "loadHomePageOnStartup");
        child.checked = p_s.startupLoadHomePage;
        
        child = findChild(p_s.widget, "connectToHomePage");
        child.checked = p_s.startupConnectToHomePage;
    },
    
    readConfig: function()
    {
        this.homepage = config.Get(this.configFile, this.urlSection, "homepage");
        if (this.homepage == null)
        {
            this.homepage = "http://www.realxtend.org/";
            config.Set(this.configFile, this.urlSection, "homepage", this.homepage);    
        }
        
        this.newTabUrl = config.Get(this.configFile, this.urlSection, "newtab");
        if (this.newTabUrl == null)
        {
            this.newTabUrl = "http://www.realxtend.org/";
            config.Set(this.configFile, this.urlSection, "newtab", this.newTabUrl);    
        }
        
        // Note: QSettings/QVariant and js booleans dont mix up too well. It will give you a string back of the config value.
        // new Boolean("false") in js will be true, so it cant be used. Inspect the string value and set the booleans right.
        // Also init write will be a string, its a stupid and confusing thing but a work around.
        this.newTabOpenHomepage = config.Get(this.configFile, this.behaviourSection, "newtab_load_homepage");
        if (this.newTabOpenHomepage == null)
        {
            this.newTabOpenHomepage = "false";
            config.Set(this.configFile, this.behaviourSection, "newtab_load_homepage", this.newTabOpenHomepage);
        }
        if (this.newTabOpenHomepage == "true")
            this.newTabOpenHomepage = true;
        else
            this.newTabOpenHomepage = false;
        
        this.startupLoadHomePage = config.Get(this.configFile, this.behaviourSection, "startup_load_homepage");
        if (this.startupLoadHomePage == null)
        {
            this.startupLoadHomePage = "true";
            config.Set(this.configFile, this.behaviourSection, "startup_load_homepage", this.startupLoadHomePage);    
        }
        if (this.startupLoadHomePage == "true")
            this.startupLoadHomePage = true;
        else
            this.startupLoadHomePage = false;
        
        this.startupConnectToHomePage = config.Get(this.configFile, this.behaviourSection, "startup_load_homeserver");
        if (this.startupConnectToHomePage == null)
        {
            this.startupConnectToHomePage = "false";
            config.Set(this.configFile, this.behaviourSection, "startup_load_homeserver", this.startupConnectToHomePage);    
        }
        if (this.startupConnectToHomePage == "true")
            this.startupConnectToHomePage = true;
        else
            this.startupConnectToHomePage = false;
    },
    
    writeConfig: function()
    {
        config.Set(this.configFile, this.urlSection, "homepage", this.homepage);
        config.Set(this.configFile, this.urlSection, "newtab", this.newTabUrl);
        config.Set(this.configFile, this.behaviourSection, "newtab_load_homepage", this.newTabOpenHomepage);
        config.Set(this.configFile, this.behaviourSection, "startup_load_homepage", this.startupLoadHomePage);
        config.Set(this.configFile, this.behaviourSection, "startup_load_homeserver", this.startupConnectToHomePage); 

        this.browserManager.toolTip = "Go to home page " + this.homepage;
    },
    
});

function HasTundraScheme(urlString)
{
    if (urlString.toLowerCase().substring(0,9) == "tundra://")
        return true;
    return false;
}

function ParseTundraLoginInfo(urlString)
{
    var loginUrl = new QUrl.fromUserInput(urlString);   
    var loginUsername = loginUrl.queryItemValue("username");
    var loginPassword = loginUrl.queryItemValue("password");
    var loginAvatarurl = loginUrl.queryItemValue("avatarurl");
    var loginProtocol = loginUrl.queryItemValue("protocol");
    var loginAddress = loginUrl.host();
    var loginPort = loginUrl.port();
    if (loginPort == -1)
        loginPort = 2345;
    if (loginProtocol == "" || loginProtocol == null)
        loginProtocol = "tcp";
        
    var loginInfo = { "username" : loginUsername,
                      "password" : loginPassword,
                      "avatarurl" : loginAvatarurl,
                      "protocol" : loginProtocol,
                      "address" : loginAddress,
                      "port" : loginPort };
    return loginInfo;
}

if (!server.IsAboutToStart() && !framework.IsHeadless())
{
    var p_ = new BrowserManager();
    p_.start();
}
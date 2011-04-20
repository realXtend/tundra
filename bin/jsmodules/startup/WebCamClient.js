
engine.ImportExtension("qt.core");
engine.ImportExtension("qt.gui");

engine.IncludeFile("jsmodules/lib/class.js");

var WebCamClient = Class.extend
({
    init: function()
    {
        this.widget = new QWidget();
        this.widget.windowFlags = Qt.Tool;
        this.widget.windowTitle = "Web Camera Client";
        this.widget.setLayout(new QVBoxLayout());
        
        var infoLabel = new QLabel("This widget will show the current feed from a webview if one is available. \nThe web cam feed is provided by CameraInputModule and all the functionality to invoke it and show the feed is in JavaScript. \nThe c++ module caps FPS to 15 at the moment to leave time for Qts/our mainloop to run smoothly aswell.\nYou can find the example code from <install_dir>/data/jsmodules/startup/WebCamClient.js.");
        infoLabel.wordWrap = true;
        infoLabel.alignment = Qt.AlignCenter;
        infoLabel.setStyleSheet("font: Calibri; font-size: 12px;");
        
        this.pictureFrame = new QLabel();
        this.toggleCaptureButton = new QPushButton("Start Capturing");
        
        this.widget.layout().addWidget(infoLabel, 0, 0);
        this.widget.layout().addWidget(this.pictureFrame, 0, 0);
        this.widget.layout().addWidget(this.toggleCaptureButton, 0, 0);
        
        this.toggleCaptureButton.clicked.connect(this.toggleCapturing);
        
        this.visibToggleAction = ui.MainWindow().AddMenuAction("&View", "Web Camera", new QIcon("./data/ui/images/communications_iconVideo.png"));
        this.visibToggleAction.triggered.connect(this.toggleWidgetVisibility);
        
        camerainput.FrameUpdate.connect(this.frameUpdated);
        camerainput.Capturing.connect(this.captureStateChanged);
    },
    
    toggleWidgetVisibility: function()
    {
        p_.widget.visible = !p_.widget.visible;
    },
    
    toggleCapturing: function()
    {
        if (camerainput.IsCapturing())
            camerainput.StopCapturing();
        else
            camerainput.StartCapturing();
    },
    
    captureStateChanged: function(capturing)
    {
        if (capturing)
            p_.toggleCaptureButton.text = "Stop Capturing";
        else
            p_.toggleCaptureButton.text = "Start Capturing";
    },
    
    frameUpdated: function(frame)
    {
        if (p_.widget.visible)
        {
            var pixmap = QPixmap.fromImage(frame);
            if (pixmap.isNull())
                return;
            var pixmapSize = pixmap.size();
            if (pixmapSize.width() <= 0 || pixmapSize.height() <= 0)
                return;
            if (p_.pictureFrame.size != pixmap.size)
            {
                p_.pictureFrame.setFixedSize(pixmap.size());
                p_.widget.resize(1,1);
            }
            p_.pictureFrame.setPixmap(pixmap);
        }
    }
});

if (!server.IsAboutToStart() && !framework.IsHeadless() && framework.GetModuleQObj("CameraInputModule") != null)
{
    var p_ = new WebCamClient();
}
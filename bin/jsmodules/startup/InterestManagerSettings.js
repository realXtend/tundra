/*
 *   For conditions of distribution and use, see copyright notice in LICENSE
 *   Implements Interest Manager widget functionality. 
 */

/*Variables used in controlling the buttons and such*/
var tundralogicmodule = null;

var isEnabled = null;
var PresetsBox = null;

var radA3 = null;
var radEA3 = null;
var radEuclidean = null;

var EuclBox = null;
var spnEuclRadius

var RayBox = null;
var spnRayInt = null;

var ModifiersBox = null;
var spnRelRadius = null;

var btnApply = null;
var btnCancel = null;

// Saves widget position to config and destroys the widget.
function OnScriptDestroyed()
{
    if (!framework.IsHeadless())
    {
        if (IMWidget)
        {
            SaveWindowPositionToSettings();
            IMWidget.deleteLater();
            IMWidget = null;
        }
    }
}

// Renderer IM widget and menu usable only in headful mode.
if (!framework.IsHeadless())
{
    engine.ImportExtension("qt.core");
    engine.ImportExtension("qt.gui");

    var configFile = "tundra";
    var configUiSection = "ui";

    var configWinPos = "im settings windows pos"
    var IMWidget = null;

    CreateMenu();
}


// Add menu entry to Settings menu
function CreateMenu()
{
    var IMMenu = findChild(ui.MainWindow().menuBar(), "SettingsMenu");

    if (IMMenu == null)
    {
        print("Settings not yet created by MenuBar. Waiting");
        frame.DelayedExecute(1.0).Triggered.connect(CreateMenu);
        return;
    }
       
    var IMSettings = IMMenu.addAction("Interest Management");
    IMSettings.triggered.connect(ShowIMSettings);
}

// Shows IM or hides it if it's already visible.
function ShowIMSettings()
{
    if (IMWidget)
    {
        IMWidget.visible = !IMWidget.visible;
        return;
    }

    CreateIMSettingsWindow();
    IMWidget.visible = true;

    LoadWindowPositionFromSettings();
}

// Creates the widget, but does not show it.
function CreateIMSettingsWindow()
{
    IMWidget = new QWidget(ui.MainWindow());
    IMWidget.setWindowFlags(Qt.Tool);
    IMWidget.setLayout(new QVBoxLayout());
    QApplication.setStyle("windows");
    var child = ui.LoadFromFile(application.installationDirectory + "data/ui/IMDialog.ui", false);
    child.setParent(IMWidget);
    IMWidget.layout().addWidget(child, 0, 0);

    IMWidget.setWindowTitle("Interest Manager Settings");

    isEnabled = findChild(IMWidget, "chkFilterEnabled");
    isEnabled.stateChanged.connect(FilteringChanged);

    btnApply  = findChild(IMWidget, "btnApply"); 
    btnApply.clicked.connect(ApplyButtonClicked);

    btnCancel = findChild(IMWidget, "btnCancel"); 
    btnCancel.clicked.connect(CancelButtonClicked);

    PresetsBox = findChild(IMWidget, "Presets");
        radA3     = findChild(IMWidget, "radA3");
        radA3.toggled.connect(A3RadioButtonToggled);
        radEA3    = findChild(IMWidget, "radEA3");
        radEA3.toggled.connect(EA3RadioButtonToggled);
        radEuclidean = findChild(IMWidget, "radEuclidean");
        radEuclidean.toggled.connect(EuclideanRadioButtonToggled);

    EuclBox = findChild(IMWidget, "EuclDistBox");
        spnEuclRadius = findChild(IMWidget, "spnERadius");
        spnEuclRadius["valueChanged(int)"].connect(EuclideanSpinBoxChanged);

    RayBox = findChild(IMWidget, "RayVisibilityBox");
        spnRayInt = findChild(IMWidget, "spnRayInt");

    ModifiersBox = findChild(IMWidget, "ModifiersBox");
        spnRelRadius = findChild(IMWidget, "spnRelRadius");
        spnRelRadius["valueChanged(int)"].connect(RelevanceSpinBoxChanged);
        spnUpdateInt = findChild(IMWidget, "spnUpdateInterval");
}

// Loads window position from config.
function LoadWindowPositionFromSettings()
{
    if (IMWidget && config.HasValue(configFile, configUiSection, configWinPos))
    {
        var pos = config.Get(configFile, configUiSection, configWinPos);
        ui.MainWindow().EnsurePositionWithinDesktop(IMWidget, pos);
    }
}

// Saves window position to config.
function SaveWindowPositionToSettings()
{
    if (IMWidget)
        config.Set(configFile, configUiSection, configWinPos, IMWidget.pos);
}

// Sets Filtering Eanbled / Disabled
function FilteringChanged(value)
{
    switch(value)
    {
        case 0:
            PresetsBox.setEnabled(false);
            EuclBox.setEnabled(false);
            RayBox.setEnabled(false);
            ModifiersBox.setEnabled(false);
        break;

        case 2:
            PresetsBox.setEnabled(true);

            if(radA3.checked)
                A3RadioButtonToggled(2);
            if(radEA3.checked)
                EA3RadioButtonToggled(2);
            if(radEuclidean.checked)
                EuclideanRadioButtonToggled(2);
        break;

        default:
        break;
    }
}

// Handles the Apply button click events
function ApplyButtonClicked(value)
{
    var enabled = isEnabled.checked;
    var eucl_e = EuclBox.enabled;
    var ray_e = RayBox.enabled;
    var rel_e = ModifiersBox.enabled;
    var critrange = spnEuclRadius.value;
    var relrange = spnRelRadius.value;
    var updateint = spnUpdateInt.value;
    var raycastint = spnRayInt.value;

    syncmanager.UpdateInterestManagerSettings(enabled, eucl_e, ray_e, rel_e, critrange, relrange, updateint, raycastint);
}

// Handles the Cancel button click events
function CancelButtonClicked(value)    
{
    IMWidget.close();
}

// Handles the A3 Radiobutton events
function A3RadioButtonToggled(value)
{
    EuclBox.setEnabled(true);
    RayBox.setEnabled(false);
    ModifiersBox.setEnabled(true);
}

// Handles the EA3 Radiobutton events
function EA3RadioButtonToggled(value)
{
    EuclBox.setEnabled(true);
    RayBox.setEnabled(true);
    ModifiersBox.setEnabled(true);
}

// Handles the Euclidean Radiobutton events
function EuclideanRadioButtonToggled(value)
{
    EuclBox.setEnabled(true);
    RayBox.setEnabled(false);
    ModifiersBox.setEnabled(false);
}

function EuclideanSpinBoxChanged(value)
{
    if((value + 1) > spnRelRadius.value)
        spnRelRadius.setValue(value + 1);
}

function RelevanceSpinBoxChanged(value)
{
    if(value < spnRayRadius.value)
        spnRayRadius.setValue(value);
    if(value < spnEuclRadius.value)
        spnEuclRadius.setValue(value);
}

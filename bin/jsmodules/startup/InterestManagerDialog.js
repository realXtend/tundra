/*
 *   For conditions of distribution and use, see copyright notice in LICENSE
 *   Implements Interest Manager widget functionality. 
 */

/*Variables used in controlling the buttons and such*/
var tundralogicmodule = null;

var moveButton = null;

var isEnabled = null;
var PresetsBox = null;

var radA3 = null;
var radEA3 = null;
var radManual = null;

var EuclBox = null;
var cbEuclEnabled 
var spnEuclRadius

var RayBox = null;
var cbRayEnabled = null;
var spnRayRadius = null;

var ModifiersBox = null;
var cbRelEnabled = null;
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

// Renderer IM widget usable only in headful mode.
if (!framework.IsHeadless())
{
    engine.ImportExtension("qt.core");
    engine.ImportExtension("qt.gui");

    var configFile = "tundra";
    var configUiSection = "ui";

    var configWinPos = "im settings windows pos"
    var IMWidget = null;

    if (framework.GetModuleByName("TundraLogic"))
        tundralogicmodule = framework.GetModuleByName("TundraLogic");
    else
        print("TundraLogicModule not in use, cannot use interest manager dialog.");
}

Initialize();
// Add menu entry to Settings menu
function Initialize()
{    
    var IMMenu = findChild(ui.MainWindow().menuBar(), "SettingsMenu");

    if (IMMenu == null)
    {
        print("Settings not yet created. Waiting");
        frame.DelayedExecute(1.0).Triggered.connect(Initialize);
        return;
    }
    
/*
    if (!IMMenu)
    {
        IMMenu = ui.MainWindow().menuBar().addMenu("&Settings");
        IMMenu.objectName = "SettingsMenu";
    }
*/    
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

    moveButton = findChild(IMWidget, "btnMove");
    moveButton.clicked.connect(MoveButtonClicked);

    btnApply  = findChild(IMWidget, "btnApply"); 
    btnApply.clicked.connect(ApplyButtonClicked);

    btnCancel = findChild(IMWidget, "btnCancel"); 
    btnCancel.clicked.connect(CancelButtonClicked);

    PresetsBox = findChild(IMWidget, "Presets");
        radA3     = findChild(IMWidget, "radA3");
        radA3.toggled.connect(A3RadioButtonToggled);
        radEA3    = findChild(IMWidget, "radEA3");
        radEA3.toggled.connect(EA3RadioButtonToggled);
        radManual = findChild(IMWidget, "radManual");
        radManual.toggled.connect(ManualRadioButtonToggled);

    EuclBox = findChild(IMWidget, "EuclDistBox");
        cbEuclEnabled = findChild(IMWidget, "chkEuclEnabled");
        cbEuclEnabled.stateChanged.connect(EuclideanCheckBoxToggled);
        spnEuclRadius = findChild(IMWidget, "spnERadius");
        spnEuclRadius["valueChanged(int)"].connect(EuclideanSpinBoxChanged);

    RayBox = findChild(IMWidget, "RayVisibilityBox");
        cbRayEnabled = findChild(IMWidget, "chkRayEnabled");
        cbRayEnabled.stateChanged.connect(RayCheckBoxToggled);
        spnRayRadius = findChild(IMWidget, "spnRayRadius");
        spnRayRadius["valueChanged(int)"].connect(RaySpinBoxChanged);

    ModifiersBox = findChild(IMWidget, "ModifiersBox");
        cbRelEnabled = findChild(IMWidget, "chkRelEnabled");
        cbRelEnabled.stateChanged.connect(RelevanceCheckBoxToggled);
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
            cbEuclEnabled.setChecked(false);
            cbRayEnabled.setChecked(false);
            cbRelEnabled.setChecked(false);
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
            if(radManual.checked)
                ManualRadioButtonToggled(2);
        break;

        default:
        break;
    }
}

// Handles the Apply button click events
function ApplyButtonClicked(value)
{
    var enabled = isEnabled.checked;
    var eucl_e = cbEuclEnabled.checked;
    var ray_e = cbRayEnabled.checked;
    var rel_e = cbRelEnabled.checked;
    var critrange = spnEuclRadius.value;
    var rayrange = spnRayRadius.value;
    var relrange = spnRelRadius.value;
    var updateint = spnUpdateInt.value;

    tundralogicmodule.InterestManagerSettingsUpdated(enabled, eucl_e, ray_e, rel_e, critrange, rayrange, relrange, updateint);
}

// Handles the Cancel button click events
function CancelButtonClicked(value)    
{
    IMWidget.close();
}

// Handles the Start Moving buttons click event
function MoveButtonClicked(value)
{
    /*Go through all entities and order them to start moving*/
    var users = server.AuthenticatedUsers();

    if (users.length > 0)
        print("[InterestManagerDialog] Ordering the avatars to start moving!");
    else
        print("[InterestManagerDialog] No avatars present inside the environment at the moment. Aborting!");

    for(var i = 0; i < users.length; i++)
    {
        var entity = framework.Scene().MainCameraScene().GetEntityByName("Avatar" + users[i].id);

        if(entity != null)
        {
            print("[InterestManagerDialog] Telling Avatar" + users[i].id + " to start moving.");
            entity.Exec(4, "StartMoving");
        }
        else
            print("[InterestManagerDialog] Something went wrong!");
    }
}

// Handles the A3 Radiobutton events
function A3RadioButtonToggled(value)
{
    cbEuclEnabled.setChecked(true);
    cbRayEnabled.setChecked(false);
    cbRelEnabled.setChecked(true);
    EuclBox.setEnabled(true);
    RayBox.setEnabled(false);
    ModifiersBox.setEnabled(true);
}

// Handles the EA3 Radiobutton events
function EA3RadioButtonToggled(value)
{
    cbEuclEnabled.setChecked(true);
    cbRayEnabled.setChecked(true);
    cbRelEnabled.setChecked(true);
    EuclBox.setEnabled(true);
    RayBox.setEnabled(false);
    ModifiersBox.setEnabled(true);
}

// Handles the Manual Radiobutton events
function ManualRadioButtonToggled(value)
{
    cbEuclEnabled.setChecked(false);
    cbRayEnabled.setChecked(false);
    cbRelEnabled.setChecked(false);
    EuclBox.setEnabled(true);
    RayBox.setEnabled(true);
    ModifiersBox.setEnabled(true);
}


function EuclideanCheckBoxToggled(value)
{
    if(value == 2)
        spnEuclRadius.setEnabled(true);
    else
        spnEuclRadius.setEnabled(false);
}

function EuclideanSpinBoxChanged(value)
{
    if((value + 1) > spnRelRadius.value)
        spnRelRadius.setValue(value + 1);
}

function RayCheckBoxToggled(value)
{
    if(value == 2)
        spnRayRadius.setEnabled(true);
    else
        spnRayRadius.setEnabled(false);
}

function RaySpinBoxChanged(value)
{
    if((value + 1) > spnRelRadius.value)
        spnRelRadius.setValue(value + 1);
}

function RelevanceCheckBoxToggled(value)
{
    if(value == 2)
        spnRelRadius.setEnabled(true);
    else
        spnRelRadius.setEnabled(false);
}

function RelevanceSpinBoxChanged(value)
{
    if(value < spnRayRadius.value)
        spnRayRadius.setValue(value);
    if(value < spnEuclRadius.value)
        spnEuclRadius.setValue(value);
}

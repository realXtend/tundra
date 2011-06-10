// !ref: local://SettingsWidget.ui

if (!framework.IsHeadless())
{
    engine.ImportExtension("qt.core");
    engine.ImportExtension("qt.gui");
    
    var mainwin = ui.MainWindow();
	var renderWidget;
	var themeWidget;
	var cave_win;
	var stereo_win;
	var delayed;
        
    // Settings
    mainwin.AddMenu("&Settings", 80);
	
	var settingsWidget = ui.LoadFromFile("./data/ui/SettingsWidget.ui", false);
	if (!settingsWidget)
		return;
	
	//Settings widget
	settingsWidget.setWindowTitle("Settings");	
	var opacitySlider = findChild(settingsWidget, "opacitySlider");
	var cancelPushButton = findChild(settingsWidget, "cancelPushButton");
	var settingsTabWidget = findChild(settingsWidget, "settingsTabWidget");
	
	opacitySlider.valueChanged.connect(OpacitySliderChanged);
	cancelPushButton.clicked.connect(Canceled);
	
	//Add it to window and to menu
	var settingsUiWidget = ui.AddWidgetToWindow(settingsWidget);
	toggle_menu = mainwin.AddMenuAction("&Settings", "General Settings");
    toggle_menu.checkable = true;
	toggle_menu.triggered.connect(settingsUiWidget, settingsUiWidget.toggleVisibility);
	settingsUiWidget.visibilityChanged.connect(toggle_menu, toggle_menu.setChecked);

	if (framework.GetModuleQObj("UI"))
	{
	
		//var languageWidget =  framework.GetModuleQObj("UI").GetLanguageSettingsWidget();
		//AddTabWidget(languageWidget, "Language");
		
		//Languaje settings
		themeWidget =  framework.GetModuleQObj("UI").GetThemeSettingsWidget();
		if (themeWidget)
			AddTabWidget(themeWidget, "Theme");
	}
			
	//OgreRenderingSettinngs
	if (framework.GetModuleQObj("OgreRendering"))
	{
		renderWidget = framework.GetModuleQObj("OgreRendering").GetRendererSettingsWidget();
		if (renderWidget)
			AddTabWidget(cave_win, "RenderSettings");
	}	
	if (framework.GetModuleQObj("CAVEStereo"))
	{
		cave_win = framework.GetModuleQObj("CAVEStereo").GetCaveWindow();
		if (cave_win)
			AddTabWidget(cave_win, "Cave");
			
		stereo_win = framework.GetModuleQObj("CAVEStereo").GetStereoscopyWindow();
		if(stereo_win)
			AddTabWidget(stereo_win, "Stereoscopy");
	}
	
	delayed = frame.DelayedExecute(5.0);
	delayed.Triggered.connect(checkForSettings);	
}

function checkForSettings()
{
	if(!renderWidget)
	{
		renderWidget = framework.GetModuleQObj("OgreRendering").GetRendererSettingsWidget();
		if (renderWidget)
			AddTabWidget(renderWidget, "RenderSettings");
	}
	if(!themeWidget)
	{
		themeWidget = framework.GetModuleQObj("UI").GetThemeSettingsWidget();
		if (themeWidget)
			AddTabWidget(themeWidget, "Theme");
	}
	
	if(!cave_win)
	{
		cave_win = framework.GetModuleQObj("CAVEStereo").GetCaveWindow();
		if (cave_win)
			AddTabWidget(cave_win, "Cave");	
	}
	
	if(!stereo_win)
	{
		stereo_win = framework.GetModuleQObj("CAVEStereo").GetStereoscopyWindow();
		if(stereo_win)
			AddTabWidget(stereo_win, "Stereoscopy");
	}
	
	if(!themeWidget || !cave_win || !stereo_win || !!renderWidget)
	{
		delayed = frame.DelayedExecute(5.0);
		delayed.Triggered.connect(checkForSettings);
	}
}
	
function AddTabWidget(widget, tab_name)
{
    settingsTabWidget.addTab(widget, tab_name);
}

function ToggleVisibility()
{
    settingsUiWidget.toggleVisibility();    
}

function OpacitySliderChanged(new_value)
{
	var opacityValueLabel = findChild(settingsWidget, "opacityValueLabel");
	opacityValueLabel.text = new_value;
}

function Canceled()
{
	ToggleVisibility();
}
	
function OpenStereoscopyWindow() {
	framework.GetModuleQObj("CAVEStereo").ShowStereoscopyWindow();
}

function OpenCaveWindow() {
	framework.GetModuleQObj("CAVEStereo").ShowCaveWindow();
}
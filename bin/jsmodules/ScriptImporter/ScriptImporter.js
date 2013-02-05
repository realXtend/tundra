// For conditions of distribution and use, see copyright notice in LICENSE   

engine.IncludeFile("lib/class.js");
engine.ImportExtension("qt.core");

var appInstallDir = QDir.fromNativeSeparators(application.installationDirectory);
var uiBase = appInstallDir + "jsmodules/ScriptImporter/ui/";

var _doNotAlterAssetRefItemText = "<Do not alter asset reference>";
var _scriptToBeImportedText = "Script to be imported: ";

var ScriptImporter = Class.extend
({
    init: function()
    {
        this.ui = {};

        this.ui.widget = ui.LoadFromFile(uiBase + "ScriptImporterWidget.ui", false);
        this.ui.addToExistingOption = findChild(this.ui.widget, "addToExistingOption");
        this.ui.assetStorageBox = findChild(this.ui.widget, "assetStorageBox");
        this.ui.createNewEntityOption = findChild(this.ui.widget, "createNewEntityOption");
        this.ui.entityListWidget = findChild(this.ui.widget, "entityListWidget");
        this.ui.closeButton = findChild(this.ui.widget, "closeButton");
        this.ui.importButton = findChild(this.ui.widget, "importButton");
        this.ui.componentNameEdit = findChild(this.ui.widget, "componentNameEdit");
        this.ui.newEntityEdit = findChild(this.ui.widget, "newEntityEdit");
        this.ui.searchEdit = findChild(this.ui.widget, "searchEdit");
        this.ui.entityNameLabel = findChild(this.ui.widget, "entityNameLabel");
        this.ui.compNameLabel = findChild(this.ui.widget, "compNameLabel");
        this.ui.localCheckBox = findChild(this.ui.widget, "localCheckBox");
        this.ui.tempCheckBox = findChild(this.ui.widget, "tempCheckBox");
        this.ui.runModeBox = findChild(this.ui.widget, "runModeBox");
        this.ui.scriptListLabel = findChild(this.ui.widget, "scriptListLabel");
        this.ui.progressBar = findChild(this.ui.widget, "progressBar");
        this.ui.statusLabel = findChild(this.ui.widget, "statusLabel");

        this.ui.optionsGroup = new QButtonGroup();
        this.ui.optionsGroup.addButton(this.ui.createNewEntityOption, 0);
        this.ui.optionsGroup.addButton(this.ui.addToExistingOption, 1);
        
        this.ui.clearButton = new QToolButton(this.ui.searchEdit);
        var iconClear = new QIcon(uiBase + "Clear.png");
        this.ui.clearButton.icon = iconClear;
        this.ui.clearButton.cursor = Qt.ArrowCursor;
        this.ui.clearButton.hide();

        this.ui.progressBar.minimum = 0;
        this.ui.progressBar.maximum = 1;
        this.ui.progressBar.value = 0;

        var frameWidth = this.ui.searchEdit.style().pixelMetric(QStyle.PM_DefaultFrameWidth);
        this.ui.searchEdit.setStyleSheet("QLineEdit#searchEdit { padding-right: " + (this.ui.clearButton.sizeHint.width() + frameWidth + 1) + "px; } ");
        this.ui.searchEdit.placeholderText = "Filter entities by name here...";
        this.ui.clearButton.setStyleSheet("QToolButton{ border: none; padding: 0px; }");

        ui.GraphicsView().DragEnterEvent.connect(this, this.handleDragEnterEvent);
        ui.GraphicsView().DragMoveEvent.connect(this, this.handleDragMoveEvent);
        ui.GraphicsView().DropEvent.connect(this, this.handleDropEvent);

        this.ui.clearButton.clicked.connect(this.ui.searchEdit, this.ui.searchEdit.clear);
        this.ui.searchEdit['textChanged(const QString&)'].connect(this, this.handleTextChanged);
        this.ui.newEntityEdit['textChanged(const QString&)'].connect(this, this.handleEntityEditChanged);
        this.ui.optionsGroup['buttonClicked(int)'].connect(this, this.handleOptionChange);
        this.ui.progressBar['valueChanged(int)'].connect(this, this.handleProgressBarChange);
        this.ui.importButton.clicked.connect(this, this.doImport);
        this.ui.closeButton.clicked.connect(this, this.doAbort);
    },

    scene: function()
    {
        return framework.Scene().MainCameraScene();
    },

    stripPath: function(path)
    {
        var filePrefix = "file://";
        return path.substring(path.lastIndexOf(filePrefix) + filePrefix.length, path.length);
    },

    removeExtension: function(filename)
    {
        return filename.substring(0, filename.lastIndexOf("."));
    },

    handleOptionChange: function(id)
    {
        var newEntitySelected = (id == 0);
        this.ui.entityNameLabel.enabled = newEntitySelected;
        this.ui.newEntityEdit.enabled = newEntitySelected;

        var existingEntitySelected = (id == 1);
        this.ui.componentNameEdit.enabled = existingEntitySelected;
        this.ui.compNameLabel.enabled = existingEntitySelected;
        this.ui.searchEdit.enabled = existingEntitySelected;
        this.ui.entityListWidget.enabled = existingEntitySelected;

        this.ui.importButton.enabled = this.data.editable && (existingEntitySelected || (newEntitySelected && this.ui.newEntityEdit.text != ""));

        if (existingEntitySelected)
            this.populateEntityList(this.ui.searchEdit.text);
    },

    handleTextChanged: function(text)
    {
        this.ui.clearButton.setVisible(text != "");
        this.populateEntityList(text);
    },

    handleEntityEditChanged: function(text)
    {
        this.ui.importButton.enabled = (text != "");
    },

    handleDragEnterEvent: function(event)
    {
        event.acceptProposedAction();
    },

    handleDragMoveEvent: function(event)
    {
        event.acceptProposedAction();
    },

    handleDropEvent: function(event, widgetUnderMouse)
    {
        if (this.ui.widget.visible)
        {
            event.acceptProposedAction();
            return;
        }

        var url = event.mimeData().urls()[0].toString();
        if (url.substring(url.lastIndexOf("."), url.length) != ".js")
            return;

        this.data = {};
        this.data.nFailure = 0;
        this.data.editable = true;
        this.ui.progressBar.value = 0;

        this.ui.createNewEntityOption.checked = true;
        this.handleOptionChange(0);
        this.populateAssetStorages();

        /// todo: should we handle more scripts per drag-dropping?
        this.data.url = this.stripPath(event.mimeData().urls()[0].toString());
        var filename = asset.ExtractFilenameFromAssetRef(this.data.url);

        this.ui.scriptListLabel.text = _scriptToBeImportedText + filename;
        this.ui.newEntityEdit.text = this.removeExtension(filename);
        this.ui.componentNameEdit.text = "";
        this.ui.localCheckBox.checked = false;
        this.ui.tempCheckBox.checked = false;
        this.ui.runModeBox.currentIndex = 0;

        this.ui.progressBar.hide();
        this.ui.statusLabel.hide();
        this.ui.widget.show();
        event.acceptProposedAction();
    },

    handleProgressBarChange: function(value)
    {
        this.printStatus();
    },

    handleOptions: function(scriptRef)
    {
        if (this.ui.createNewEntityOption.checked)
            this.createNewEntity(this.ui.newEntityEdit.text, scriptRef);
        else if (this.ui.addToExistingOption.checked)
            this.addCompToEntity(this.ui.entityListWidget.currentItem().text(), this.ui.componentNameEdit.text, scriptRef);
    },

    populateEntityList: function(searchString)
    {
        this.ui.entityListWidget.clear();

        var entityList = new Array();
        var entities = this.scene().FindEntities("*" + searchString + "*");

        for (var i = 0; i < entities.length; i++)
            entityList.push(entities[i].Name());

        this.ui.entityListWidget.insertItems(0, entityList);
    },

    populateAssetStorages: function()
    {
        this.ui.assetStorageBox.clear();

        var index = 0;
        var assetStorages = asset.GetAssetStorages();
        var defaultAssetStorage = asset.GetDefaultAssetStorage();

        this.ui.assetStorageBox.insertItem(index++, _doNotAlterAssetRefItemText);

        for (var i = 0; i < assetStorages.length; i++)
            if (assetStorages[i].Writable())
                this.ui.assetStorageBox.insertItem(index++, assetStorages[i].Name());
    },

    doImport: function()
    {
        if (!this.validateEntityName(this.ui.newEntityEdit.text))
            return;
        if (!this.validateComponentName(this.ui.componentNameEdit.text))
            return;

        this.data.editable = false;
        this.ui.importButton.enabled = this.data.editable;

        if (this.ui.assetStorageBox.currentText == _doNotAlterAssetRefItemText)
        {
            var filename = asset.ExtractFilenameFromAssetRef(this.data.url);
            //var path = this.data.url.substring(0, this.data.url.lastIndexOf(filename) - 1);
            this.ui.progressBar.value++;

            this.handleOptions(filename);
        }
        else
        {
            var transfer = asset.UploadAssetFromFile(this.data.url, this.ui.assetStorageBox.currentText);
            if (transfer != null)
            {
                transfer.Completed.connect(this, this.onAssetUploadCompleted);
                transfer.Failed.connect(this, this.onAssetUploadFailed);
            }
        }
    },

    doAbort: function()
    {
        this.ui.widget.close();
    },

    validateEntityName: function(entityName)
    {
        if (!this.ui.createNewEntityOption.checked)
            return true;

        var entity = this.scene().EntityByName(this.ui.newEntityEdit.text);
        if (entity != null)
        {
            this.showTooltip(this.ui.newEntityEdit, "An entity with this name already exists");
            return false;
        }

        return true;
    },

    validateComponentName: function(compName)
    {
        if (!this.ui.addToExistingOption.checked)
            return true;

        if (this.ui.entityListWidget.currentRow == -1)
        {
            this.showTooltip(this.ui.entityListWidget, "You must select a target entity from the list");
            return false;
        }

        var entity = this.scene().EntityByName(this.ui.entityListWidget.currentItem().text());
        if (entity != null)
        {
            var component = entity.GetComponent("EC_Script", compName);
            if (component != null)
            {
                this.showTooltip(this.ui.componentNameEdit, "A component with this name already exists in the target entity");
                return false;
            }
        }
        else
        {
            // should not come here
            this.showTooltip(this.ui.entityListWidget, "The selected entity is not valid anymore, perhaps it was removed. Please select another one from the list");
            return false;
        }

        return true;
    },

    showTooltip: function(widget, text)
    {
        QToolTip.showText(widget.mapToGlobal(new QPoint(1,1)), text);
    },

    createNewEntity: function(entityName, scriptRef)
    {
        var entity = null;
        if (this.ui.localCheckBox.checked)
            entity = this.scene().CreateLocalEntity(["EC_Name", "EC_Script"]);
        else
            entity = this.scene().CreateEntity(this.scene().NextFreeId(), ["EC_Name", "EC_Script"]);
            
        entity.SetName(entityName);
        entity.temporary = this.ui.tempCheckBox.checked;

        this.addCompToEntity(entityName, "", scriptRef);
    },

    addCompToEntity: function(entityName, compName, scriptRef)
    {
        var isLocal = this.ui.localCheckBox.checked;
        var isTemp = this.ui.tempCheckBox.checked;
        var entity = this.scene().EntityByName(entityName);
        if (entity == null)
        {
            console.LogWarning("[Script importer]: Requested entity with name " + entityName + " is null! This should not happen");
            return;
        }

        var scriptComp = entity.GetOrCreateComponent("EC_Script", compName, isLocal ? 2 : 0, !isLocal);
        scriptComp.temporary = isTemp;
        scriptComp.runMode = this.ui.runModeBox.currentIndex;
        scriptComp.runOnLoad = true;

        var sRef = scriptComp.scriptRef;
        sRef[0] = scriptRef;
        scriptComp.scriptRef = sRef;
    },

    onAssetUploadCompleted: function(transfer)
    {
        this.ui.progressBar.value++;
        var scriptRef = transfer.DestinationName();

        this.handleOptions(scriptRef);
    },

    onAssetUploadFailed: function(transfer)
    {
        this.data.nFailure++;
        this.printStatus();
    },

    printStatus: function()
    {
        var value = this.ui.progressBar.value;
        if (value + this.data.nFailure == this.ui.progressBar.maximum)
        {
            if (value == this.ui.progressBar.maximum)
                this.ui.statusLabel.text = "All scripts were imported successfully";
            else
                this.ui.statusLabel.text = "Importing failed on some scripts";
            this.ui.statusLabel.show();
        }
    }
});


if (!framework.IsHeadless())
{
    engine.ImportExtension("qt.gui");
    var _p = new ScriptImporter();
}
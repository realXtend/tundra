engine.ImportExtension("qt.core");
engine.ImportExtension("qt.gui");

function SunspiderTest(controller, name, table, debug, multi, action, iter)
{
	this.controller = controller;
	this.name = name;
	this.table = table;
	this.iter = iter;
	this.action = action;
	
	this.entity = scene.CreateEntityRaw(scene.NextFreeIdLocal(), ["EC_Script", "EC_DynamicComponent"]);
	this.entity.SetTemporary(true);
	this.entity.SetName(this.name);

	scene.EmitEntityCreatedRaw(this.entity);
	
	var script = this.entity.script;
	var dynamic = this.entity.GetComponentRaw("EC_DynamicComponent");
	
	dynamic.CreateAttribute("string", "script");
	dynamic.SetAttribute("script", "./scenes/sunspider/" + this.name + ".js");
	
	if (multi)
	{
		script.runOnLoad = true;
		dynamic.CreateAttribute("uint", "controller");
		dynamic.SetAttribute("controller", me.Id);

	}
	else
	{
		script.runOnLoad = false;
		script.ScriptAssetChanged.connect(this, this.OnScriptAssetChanged);
	}

	
	if (debug)
	{
		dynamic.CreateAttribute("bool", "debug");
		dynamic.SetAttribute("debug", true);
		
	}

	
	script.type = "js";
	var r = script.scriptRef;
	r.ref = "sunspiderwrapper.js";
	script.scriptRef = r;
}


SunspiderTest.prototype.OnScriptAssetChanged = function(asset)
{

	var time = 0;	
	
	for (j = 0; j < this.iter; j++) {
		var time1 = Date.now();
		if(this.action)
			this.entity.Exec(1,"RunScript");
		else
			this.entity.script.Run();
		var time2 = Date.now();
		time += time2 - time1;
	}

	time = time/this.iter;
	
	this.controller.OnTestFinished(this.entity.Id, time);
	
};

function SunspiderController()
{
	this.tests = [ "3d-cube", "3d-morph", "3d-raytrace", "access-binary-trees", "access-fannkuch", "access-nbody", "access-nsieve", "bitops-3bit-bits-in-byte", "bitops-bits-in-byte", "bitops-bitwise-and", "bitops-nsieve-bits", "controlflow-recursive", "crypto-aes", "crypto-md5", "crypto-sha1", "date-format-tofte", "date-format-xparb", "math-cordic", "math-partial-sums", "math-spectral-norm", "regexp-dna", "string-base64", "string-fasta", "string-tagcloud", "string-unpack-code", "string-validate-input"];
	
	if (framework.IsHeadless())
	{
		this.table = null;
		print("Sunspider test STARTED");
		this.performTest();
	}
	else
	{
		this.widget = uiservice.LoadFromFile("./scenes/sunspider/sunspider.ui", false);
		this.table = findChild(this.widget, "tableResults");
		
		this.buttonCancel = findChild(this.widget, "buttonCancel");
		this.buttonCancel.clicked.connect(this, this.onFinish);
	
		this.buttonDo = findChild(this.widget, "buttonDo");
		this.buttonDo.clicked.connect(this, this.performTest);
		
		this.checkMulti = findChild(this.widget, "checkMulti");
		this.spiniter = findChild(this.widget, "spinIterations");
		this.checkAction = findChild(this.widget, "checkAction");
		
		this.checkMulti.stateChanged.connect(this, this.multiChanged);
		this.checkMulti.checked = true;
		this.spiniter.setEnabled(false);
		this.checkAction.setEnabled(false);

		uiservice.AddWidgetToScene(this.widget, false, true);
		uiservice.ShowWidget(this.widget);
	}
	
	this.performedTests = 0;
	
	me.Action("TestFinished").Triggered.connect(this, this.OnTestFinished);
}

SunspiderController.prototype.performTest = function()
{
	this.performedTests = 0;
	
	var debug = false;
	var multi = true;
	var action = false;
	var iter = 1;
	
	if (!framework.IsHeadless()) 
	{
		this.buttonDo.setEnabled(false);
		this.buttonCancel.setEnabled(false);
	
		this.table.clearContents();
		this.table.rowCount = 0;
	
		debug = findChild(this.widget, "checkDebug").checked;
		multi = this.checkMulti.checked;
		iter = this.spiniter.value;
		action = this.checkAction.checked;
	}

	for (i in this.tests)
	{
		new SunspiderTest(this, this.tests[i], this.table, debug, multi, action, iter);
	}
};

SunspiderController.prototype.OnTestFinished = function(id, time)
{
	var name = scene.GetEntityRaw(id).GetName();
	
	if (this.table == null)
	{
		print ("Sunspider Test " + name + ": " + time + "ms");
	}
	else
	{
		var row = this.table.rowCount;
		this.table.insertRow(row);
		this.table.setItem(row,0, new QTableWidgetItem(name));
		if (!this.checkMulti.checked && this.spiniter.value > 1)
			this.table.setItem(row,1, new QTableWidgetItem(Number(this.spiniter.value) + " times"));
		else
			this.table.setItem(row,1, new QTableWidgetItem("1 time"));
		this.table.setItem(row,2, new QTableWidgetItem(Number(time) + "ms"));
	}
	
	scene.RemoveEntityRaw(id);
	
	this.performedTests++;
	
	if (this.performedTests == this.tests.length)
	{
		if (framework.IsHeadless())
		{
			print("Sunspider test FINISHED");
		}
		else
		{
			this.buttonCancel.text = "Close";
			this.buttonDo.setEnabled(true);
			this.buttonCancel.setEnabled(true);
		}
	}

};

SunspiderController.prototype.onFinish = function()
{
	this.widget.close();
};

SunspiderController.prototype.multiChanged = function(state)
{
	if (state > 0)
	{
		this.spiniter.setEnabled(false);
		this.checkAction.setEnabled(false);
	}
	else
	{
		this.spiniter.setEnabled(true);
		this.checkAction.setEnabled(true);
	}
};


new SunspiderController();


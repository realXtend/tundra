engine.ImportExtension("qt.core");

//Wait for tests to finish
var jasmine_reporter = 0;
var jasmine_reporters_list = jasmine.getEnv().reporter.subReporters_;
for (i=0; i < jasmine_reporters_list.length; i++)
{
	if ("finished" in jasmine_reporters_list[i])
	{
		jasmine_reporter = jasmine_reporters_list[i];
		break;
	}
};

if (jasmine_reporter)
	JasmineCheckReporterFinished();
else
	JasmineRecursivelyFindTests();

function JasmineCheckReporterFinished()
{
	if (!jasmine_reporter.finished)
		frame.DelayedExecute(1).Triggered.connect(JasmineCheckReporterFinished);
	else
	{
		JasmineRecursivelyFindTests();
	}
};

//Recursively Find Test txmls
function JasmineRecursivelyFindTests()
{
	var name = me.name.name;
	var mainTest = false;
	var test_dir;
	if (name.substring(0,15) == "JasmineTestPath")
		test_dir = name.substring(16);
	else
	{
		me.name.name = "JasmineTestManager";
		me.Action("JasmineTestFinished").Triggered.connect(JasmineTestFinished);
		test_dir = asset.assetdir;
		mainTest = true;
	}
	
	jasmine.getGlobal().console.log("Jasmine Recursive test search on " + test_dir);
	var dir = new QDir(test_dir);
	if (dir.exists())
	{
		var subdirs = dir.entryList(["*"], QDir.Dirs, QDir.NoSort);
		for (i=0; i < subdirs.length; i++)
		{
			if (subdirs[i] == "." || subdirs[i] == "..")
				continue;
			if (!dir.cd(subdirs[i]))
				continue;
			jasmine.getGlobal().console.log("Jasmine searching for tests on " + dir.path());
			var test_list = dir.entryList(["*Test.txml"], QDir.Files, QDir.NoSort);
			for (j=0; j < test_list.length; j++)
			{
				jasmine.getGlobal().console.log("Loading Jasmine Test " + test_list[j]);
				var entities = scene.LoadSceneXML(dir.filePath(test_list[j]), false, false, 0);
				for (k=0; k < entities.length; k++)
				{
					if ("script" in entities[k])
						entities[k].name.name = "JasmineTestPath=" + dir.path();
				}
			}
			dir.cdUp();
		}
	}
	
	//Remove test entities
	if (!mainTest)
	{
		scene.GetEntity("JasmineTestManager").Exec(1,"JasmineTestFinished", me.Id);
	}
}

var test_entities_to_remove = [];

function JasmineTestFinished(id)
{
	test_entities_to_remove.push(id);
	frame.DelayedExecute(1).Triggered.connect(JasmineDoEntityRemoval);
}

function JasmineDoEntityRemoval()
{
	var entity = test_entities_to_remove.pop();
	jasmine.getGlobal().console.log("Jasmine removing test entity " + entity);
	scene.RemoveEntityRaw(entity);
}

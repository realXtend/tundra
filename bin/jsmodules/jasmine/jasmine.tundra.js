//Include jasmine and junit reporter
engine.IncludeFile("local://jasmine.js");
engine.IncludeFile("local://jasmine.junit_reporter.js");

//Required extension
engine.ImportExtension("qt.core");

//Overwrite jasmine setTimeout to make use of Tundra FrameAPI
jasmine.Env.prototype.setTimeout = function(caller, time) {
	var timer = frame.DelayedExecute(time/1000);
	timer.Triggered.connect(caller);
};

//Set jasmine Console
jasmine.TundraConsole = function() { 
	this.tundra_console = console;
	};
	
jasmine.TundraConsole.prototype.log = function(str) {
		if (framework.IsHeadless())
			print(str);
		else
			this.tundra_console.Print(str);
	};

jasmine.getGlobal().console = new jasmine.TundraConsole();

//Overwrite JUnitXmlReporter writeFile method to make use of Qt classes.
jasmine.JUnitXmlReporter.prototype.writeFile = function(filename, text) {
    try {
	    var file = new QFile(filename);
	    if (file.open(QIODevice.WriteOnly))
	    {
	    	var out = new QTextStream(file);
	    	out.writeString(text);
	    	file.close();
	    }
	    else
	    {
	    	this.log("Error opening file with write permission: " + filename);
	    }
    } catch (e) {
    	this.log("Error " + e.description + " writing to "+ filename + ". Be sure that the directory exists.");
    }
};
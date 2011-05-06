//Jasmine
engine.IncludeFile("local://jasmine.tundra.js");

//Test Specs
engine.IncludeFile("local://Vector3dfSpec.js");
engine.IncludeFile("local://QuaternionSpec.js");

//Configure reporter and execute. Path where xml reports will be stored must exist.
jasmine.getEnv().addReporter(new jasmine.JUnitXmlReporter("./jasmine_reports/"));
jasmine.getEnv().execute();

//Recursively find tests
engine.IncludeFile("local://jasmine.tundra.recursive.js");

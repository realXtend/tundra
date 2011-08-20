//Jasmine
engine.IncludeFile("local://jasmine.tundra.js");

//Test specs
engine.IncludeFile("local://RigidBodyControllerSpec.js");

//Configure reporter and execute. Path where xml reports will be stored must exist.
jasmine.getEnv().addReporter(new jasmine.JUnitXmlReporter("./jasmine_reports/world/"));
jasmine.getEnv().execute();

//Recursively find tests
engine.IncludeFile("local://jasmine.tundra.recursive.js");
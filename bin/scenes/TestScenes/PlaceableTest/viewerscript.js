// http://www.realxtend.org/doxygen/_tundra_script_reference.html

var randomnumber=Math.floor(Math.random()*100);

function AutoConnect(){
    var isserver = server.IsRunning() || server.IsAboutToStart();
    if(isserver==false){
        client.Login("localhost",2345,"Guest"+randomnumber.toString()," ","udp");
        engine.IncludeFile("scenes/TestScenes/PlaceableTest/createmovekilltest.js");
    }
    if(isserver==true){
        print("server");
        print("This script should be run as a parameter for the client.");
    }
}

frame.DelayedExecute(1).Triggered.connect(this,AutoConnect);

engine.ImportExtension("qt.core");

//none of the below is allowed for remote code with the default sandboxing

try {
    var home = QDir.home();
    print("Your home directory is at: " + home.path());
    print("And has these files:" + home.entryList());
    print("--luckily untrusted code from the net can not read this info.");
} catch (ReferenceError) {
    print("QDir not there. Running an untrusted remote code, right?");
}

try {
    p = new QProcess();
    p.start("ls /");
    out = p.readAll();
    //print(out);
    print("executed a process.");
} catch (ReferenceError) {
    print("QProcess not there. Running an untrusted remote code, right?");
}

engine.ImportExtension("qt.network");
s = new QTcpSocket();
s.connectToHost("www.realxtend.orge", 80);
s.disconnectFromHost();
print("connected to a remote host, and disconnected");

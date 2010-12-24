engine.ImportExtension("qt.core");

var home = QDir.home();
print("Your home directory is at: " + home.path());
print("And has these files:" + home.entryList());

print("--luckily untrusted code from the net can not read this info.");

p = new QProcess();
p.start("ls /");
out = p.readAll();
//print(out);
// For conditions of distribution and use, see copyright notice in LICENSE

engine.ImportExtension("qt.core");
engine.ImportExtension("qt.xml");

QtXmlTest();

// Loads the plugin.xml from Tundra/bin twice, first by using both plain QFile, and the second time using QFile and QXmlInputSource.
function QtXmlTest()
{
    var file = new QFile("plugins.xml");
    var errorMsg;
    var errorLine;
    var errorColumn;
    var domDocFromFile = new QDomDocument();
 
    // The following line should work to my knowledge, but for some (non-apparent) reason it doens't:
    // var fileOk = domDocFromFile["setContent(QIODevice,bool,QString,int,int)"](file, true, errorMsg, errorLine, errorColumn);
    // I'm suspecting it has something to do with the fact that QDomDocument doesn't inherit QObject (I have encountered some mysterious
    // behavior with e.g. QTreeWidgetItem also), and the fact that the actual C++ function implementation uses default argument values:
    // bool setContent(QIODevice *dev, bool namespaceProcessing, QString * errorMsg = 0, int * errorLine = 0, int * errorColumn = 0);
    // My guess is that that passing C-type pointers (or references for that matter) which are used for output is not simply
    // supported/understood by QtScript(Generator).
    // Probably QtScriptGenerator doesn't generate function "signatures" for all cases (the function at hand would yield four different possible signatures).
    // Hence the function call must be done with none of the "optional" arguments provided.
    // So, the following works without problems (let's try "bool setContent(QIODevice * dev)"):
    var fileOk = domDocFromFile.setContent(file);
    if (fileOk)
    {
        // Should work ok!
        // Simply dump the whole file contents to stdout.
        print("File contents:");
        print(domDocFromFile.toString());
        print("");
    }
 
    file.close(); // Remember to close the file handle.
 
    // So, the setContent function seems to be working by omitting the default arguments from the signatures
    // (http://developer.qt.nokia.com/doc/qt-4.7/qdomdocument.html) alltogether, and the following signatures/overloads should work:
    // bool setContent(const QByteArray & data, bool namespaceProcessing)
    // bool setContent(const QString & text, bool namespaceProcessing)
    // bool setContent(QIODevice * dev, bool namespaceProcessing)
    // bool setContent(QXmlInputSource * source, bool namespaceProcessing)
    // bool setContent(const QString & text)
    // bool setContent(const QByteArray & buffer)
    // bool setContent(QIODevice * dev) - which we usesd previously
    // bool setContent(QXmlInputSource * source, QXmlReader * reader)
 
    var file2 = new QFile("plugins.xml");
    var source = new QXmlInputSource(file2);
    var domDocFromInputSource = new QDomDocument();
 
    // Utilize the Qt XML/DOM API and print the names of the C++ plugins defined int the XML file individually.
    // Let's try "bool setContent(QXmlInputSource * source, bool namespaceProcessing)".
    var sourceOk = domDocFromInputSource.setContent(source, true);
    if (sourceOk)
    {
        // Should work ok!
        var pluginElems = domDocFromInputSource.elementsByTagName("plugin");
        print(pluginElems.length() + " C++ plugins found. The plugins are:");
        for(var i = 0; i < pluginElems.length(); ++i)
            print("* " + pluginElems.item(i).toElement().attribute("path"));
    }
 
    file2.close();
}

print("< JS qt.xml example:");

//converted the c++ in http://doc.trolltech.com/4.6/qdomdocument.html
var doc = new QDomDocument("mydocument");

//works when running from Tundra bin: ./Tundra --headless --run jsmodules/apitest/qtxml.js
var file = new QFile("plugins.xml"); 

//if (!file.open(QIODevice::ReadOnly))
//     return;
/*if (!doc.setContent(&file)) {
     file.close();
     //return;
}*/
doc.setContent(file);
file.close();

print("---");

// print out the element names of all elements that are direct children
 // of the outermost element.
var docElem = doc.documentElement();

var n = docElem.firstChild();
while(!n.isNull()) {
    //print(".");
    var e = n.toElement(); // try to convert the node to an element.
    if(!e.isNull()) {
        print(e.tagName()); // the node really is an element.
     }
     n = n.nextSibling();
 }

/*
 // Here we append a new element to the end of the document
 QDomElement elem = doc.createElement("img");
 elem.setAttribute("src", "myimage.png");
 docElem.appendChild(elem);
*/

/* creating - no worky for some reason?
is from http://developer.qt.nokia.com/forums/viewthread/12949 where they suspect appendChild is broken.

var doc = new QDomDocument();
var root = doc.createElement("root");
doc.appendChild(root);
root.setAttribute("attr1", "value1")
 
var tag = doc.createElement("tag1");
root.appendChild(tag);

print(doc);
print((doc.toString()));
*/

print(".. /qt xml example end> ");
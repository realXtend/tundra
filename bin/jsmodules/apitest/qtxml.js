engine.ImportExtension("qt.core");
engine.ImportExtension("qt.xml");

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
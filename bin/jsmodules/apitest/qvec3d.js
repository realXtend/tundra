//test using QVector3D in js, as am having probs with it in TOY tools on linux now for some reason
engine.ImportExtension("qt.core");
engine.ImportExtension("qt.gui");

w = new QWidget();
print("qwidget at least works, right?: " + w.width);

v = new QVector3D(1, 0, 0);
print("I'm a vector, pointing forward!: " + v);
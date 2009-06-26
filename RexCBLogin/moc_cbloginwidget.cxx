/****************************************************************************
** Meta object code from reading C++ file 'cbloginwidget.h'
**
** Created: Fri 26. Jun 10:15:14 2009
**      by: The Qt Meta Object Compiler version 61 (Qt 4.5.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "cbloginwidget.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'cbloginwidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 61
#error "This file was generated using the moc from 4.5.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_CBLoginWidget[] = {

 // content:
       2,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   12, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors

 // signals: signature, parameters, type, tag, flags
      15,   14,   14,   14, 0x05,

 // slots: signature, parameters, type, tag, flags
      39,   14,   14,   14, 0x0a,
      56,   14,   14,   14, 0x0a,
      78,   70,   14,   14, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_CBLoginWidget[] = {
    "CBLoginWidget\0\0loginProcessed(QString)\0"
    "showLoginPanel()\0showWebView()\0success\0"
    "processLogin(bool)\0"
};

const QMetaObject CBLoginWidget::staticMetaObject = {
    { &QFrame::staticMetaObject, qt_meta_stringdata_CBLoginWidget,
      qt_meta_data_CBLoginWidget, 0 }
};

const QMetaObject *CBLoginWidget::metaObject() const
{
    return &staticMetaObject;
}

void *CBLoginWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_CBLoginWidget))
        return static_cast<void*>(const_cast< CBLoginWidget*>(this));
    return QFrame::qt_metacast(_clname);
}

int CBLoginWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QFrame::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: loginProcessed((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 1: showLoginPanel(); break;
        case 2: showWebView(); break;
        case 3: processLogin((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void CBLoginWidget::loginProcessed(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE

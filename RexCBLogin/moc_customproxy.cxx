/****************************************************************************
** Meta object code from reading C++ file 'customproxy.h'
**
** Created: Fri 26. Jun 10:15:10 2009
**      by: The Qt Meta Object Compiler version 61 (Qt 4.5.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "customproxy.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'customproxy.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 61
#error "This file was generated using the moc from 4.5.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_CustomProxy[] = {

 // content:
       2,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   12, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors

 // slots: signature, parameters, type, tag, flags
      18,   13,   12,   12, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_CustomProxy[] = {
    "CustomProxy\0\0step\0updateShowStep(qreal)\0"
};

const QMetaObject CustomProxy::staticMetaObject = {
    { &QGraphicsProxyWidget::staticMetaObject, qt_meta_stringdata_CustomProxy,
      qt_meta_data_CustomProxy, 0 }
};

const QMetaObject *CustomProxy::metaObject() const
{
    return &staticMetaObject;
}

void *CustomProxy::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_CustomProxy))
        return static_cast<void*>(const_cast< CustomProxy*>(this));
    return QGraphicsProxyWidget::qt_metacast(_clname);
}

int CustomProxy::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QGraphicsProxyWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: updateShowStep((*reinterpret_cast< qreal(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 1;
    }
    return _id;
}
QT_END_MOC_NAMESPACE

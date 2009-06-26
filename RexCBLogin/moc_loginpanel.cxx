/****************************************************************************
** Meta object code from reading C++ file 'loginpanel.h'
**
** Created: Fri 26. Jun 10:15:05 2009
**      by: The Qt Meta Object Compiler version 61 (Qt 4.5.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "loginpanel.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'loginpanel.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 61
#error "This file was generated using the moc from 4.5.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_LoginPanel[] = {

 // content:
       2,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   12, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors

 // slots: signature, parameters, type, tag, flags
      12,   11,   11,   11, 0x0a,
      28,   11,   11,   11, 0x0a,
      53,   11,   45,   11, 0x0a,
      67,   11,   45,   11, 0x0a,
      86,   82,   11,   11, 0x0a,
     111,   82,   11,   11, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_LoginPanel[] = {
    "LoginPanel\0\0setProperties()\0"
    "connectSignals()\0QString\0getWorldUri()\0"
    "getOpenIDUri()\0uri\0setWorldAddress(QString)\0"
    "setOpenIDAddress(QString)\0"
};

const QMetaObject LoginPanel::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_LoginPanel,
      qt_meta_data_LoginPanel, 0 }
};

const QMetaObject *LoginPanel::metaObject() const
{
    return &staticMetaObject;
}

void *LoginPanel::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_LoginPanel))
        return static_cast<void*>(const_cast< LoginPanel*>(this));
    return QWidget::qt_metacast(_clname);
}

int LoginPanel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: setProperties(); break;
        case 1: connectSignals(); break;
        case 2: { QString _r = getWorldUri();
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = _r; }  break;
        case 3: { QString _r = getOpenIDUri();
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = _r; }  break;
        case 4: setWorldAddress((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 5: setOpenIDAddress((*reinterpret_cast< QString(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 6;
    }
    return _id;
}
QT_END_MOC_NAMESPACE

// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "PythonHandler.h"

#include <EC_OpenSimPrim.h>

#include <QDebug>

namespace WorldBuilding
{
    PythonHandler::PythonHandler(QObject *parent) :
        QObject(parent)
    {
        connect(this, SIGNAL(HelloFromPython()), SLOT(HelloPython()));
    }

    PythonHandler::~PythonHandler()
    {
    }

    void PythonHandler::HelloPython()
    {
        qDebug() << "Hello from python!";
    }

    void PythonHandler::Selected(EC_OpenSimPrim *prim)
    {
        qDebug() << "Name : " << prim->getName() << endl
                 << "ID   : " << prim->getFullId() << endl
                 << "Desc : " << prim->getDescription();
    }
}
// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_ScriptServiceInterface_h
#define incl_Interfaces_ScriptServiceInterface_h

#include "ServiceInterface.h"
#include <QString>
#include <QVariant>

namespace Foundation
{
    class ScriptServiceInterface : public ServiceInterface
    {
    public:
        ScriptServiceInterface() {}
        virtual ~ScriptServiceInterface() {}

        virtual void RunScript(const QString &scriptname) = 0;
        //virtual void RunString(QString codest, QMap<QString, QObject*> context = QMap<QString, QObject*>()) = 0;
        virtual void RunString(const QString &codestr, const QVariantMap &context = QVariantMap()) = 0;

        //virtual ScriptObject* LoadScript(const std::string& scriptname, std::string& error) = 0;
        //virtual ScriptObject* GetObject(const ScriptObject& script, const std::string& objectname, std::string& error) = 0;

    };

    /* deprecated: PythonQT and QObject mechanism is used instead. the old gtk using comms module used this. Webdav inventory uses PythonQt to implement a c++ module in py.
    class ScriptObject
    {
    public:
        ScriptObject() {}
        virtual ~ScriptObject() {}
        virtual void ReleaseResources() = 0;

        // for syntax parameter see http://docs.python.org/c-api/arg.html
        virtual ScriptObject* CallMethod(std::string& methodname, std::string& syntax, char* argv[]) = 0;
        virtual ScriptObject* CallMethod2(std::string& methodname, std::string& syntax, ...) = 0;
        virtual ScriptObject* CallMethod(std::string& methodname, const std::string& syntax, const ScriptObject* args) = 0;
        virtual ScriptObject* GetObject(const std::string& objectname) = 0;

        virtual char* ConvertToChar() = 0;
    };

    typedef boost::shared_ptr<ScriptObject> ScriptObjectPtr;

    class ScriptEventInterface
    {
    public:
        ScriptEventInterface() {}
        virtual ~ScriptEventInterface() {}
        virtual void SetCallback(void(*f)(char*), std::string key) = 0;
        virtual void NotifyScriptEvent(const std::string& key, const std::string& message) = 0;        
    };    
    */
}

#endif


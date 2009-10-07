#ifndef incl_PythonScriptObject_Script_h
#define incl_PythonScriptObject_Script_h



namespace PythonScript
{

    class PythonScriptObject : public Foundation::ScriptObject
    {
    public:
        PythonScriptObject(void);
        virtual ~PythonScriptObject(void);
        virtual void ReleaseResources();   
        virtual ScriptObject* CallMethod(std::string& methodname, std::string& syntax, char* argv[]);
        virtual ScriptObject* CallMethod2(std::string& methodname, std::string& syntax, ...);
        virtual ScriptObject* CallMethod(std::string& methodname, const std::string& syntax, const ScriptObject* args);
        virtual ScriptObject* GetObject(const std::string& objectname);

        virtual char* ConvertToChar();

        PyObject* pythonRef; // reference to class or python script
        PyObject* pythonObj; // object instance
    
    private:
        

    };
}

#endif
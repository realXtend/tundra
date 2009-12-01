#include "PythonScriptModule.h" //for the staticframework hack, for entityptr re-get workaround
#include "Entity.h"

#include "RexLogicModule.h" //for getting prim component data

/* this is not a module but defines a type for the module created in PythonScriptModule */
//initentity(void) 

namespace PythonScript
{
    static PyMethodDef entity_methods[] = {
        {NULL}  /* Sentinel */
    };

    void entity_init(PyObject* m)
    {
        rexviewer_EntityType.tp_new = PyType_GenericNew;
        if (PyType_Ready(&rexviewer_EntityType) < 0)
        {
            //std::cout << "PythonScriptModule: EntityType not ready?" << std::endl;
            PythonScript::self()->LogDebug("PythonScriptModule: EntityType not ready?");
            return;
        }

        Py_INCREF(&rexviewer_EntityType);
        PyModule_AddObject(m, "Entity", (PyObject *)&rexviewer_EntityType);
    }

    PyObject* entity_create(Core::entity_id_t ent_id) //, Scene::EntityPtr entity)
    {
        rexviewer_EntityObject* eob;
        //std::cout << "Entity: creating a wrapper pyobject ..";
        eob = PyObject_New(rexviewer_EntityObject, &rexviewer_EntityType); //sets refcount to 1

        //std::cout << "setting the pointer to the entity in the wrapper: " << entity << std::endl;
        //eob->entity = entity; //doesn't have a constructor, just this factory
        
        //std::cout << "storing the pointer to the entity in the entity_ptrs map:" << entity << std::endl;
        //entity_ptrs[ent_id] = entity;

        //std::cout << "storing the entity id in the wrapper object:" << ent_id << std::endl;
        PythonScript::self()->LogDebug("Storing the entity id in the wrapper object:" + QString::number(ent_id).toStdString());
        eob->ent_id = ent_id;
        return (PyObject*) eob;
    }

    //getattro and setattro should be here but are now in PythonScriptModule as a workaround for staticframework being null here
}

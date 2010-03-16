#ifndef incl_PythonEntityType_h
#define incl_PythonEntityType_h

#ifdef PYTHON_FORCE_RELEASE_VERSION
  #ifdef _DEBUG
    #undef _DEBUG
    #include <Python.h>
    #define _DEBUG
  #else
    #include <Python.h>
  #endif 
#else
    #include <Python.h>
#endif

#include "EntityComponent/EC_OpenSimPrim.h"
#include "EC_OgrePlaceable.h"

namespace PythonScript
{
    class PyEntity
    {
    public:
        PyObject_HEAD
        /* Type-specific fields go here. */
        //Scene::EntityPtr entity;
        //smart_ptrs can't be just like this in pyobjects, see e.g. http://wiki.python.org/moin/boost.python/PointersAndSmartPointers
        entity_id_t ent_id;
    };

    /// Registers the rex entity type into the given namespace. Called only once at startup.
    void entity_init(PyObject* pyNamespace);

    //void entity_deinit(); ///\todo

    /// Allocates a new rex entity wrapper and returns a pointer to it.
    PyObject* entity_create(entity_id_t ent_id);

    PyTypeObject *GetRexPyTypeObject();

    //void entity_delete(PyObject *obj);
}

#endif

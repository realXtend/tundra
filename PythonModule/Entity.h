#ifndef incl_PythonEntityType_h
#define incl_PythonEntityType_h

#include "Foundation.h"
#ifdef PYTHON_FORCE_RELEASE_VERSION
  #ifdef _DEBUG
    #undef _DEBUG
    #include "Python.h"
    #define _DEBUG
  #else
    #include "Python.h"
  #endif 
#else
    #include "Python.h"
#endif

#include "EntityComponent/EC_OpenSimPrim.h"
#include "EC_OgrePlaceable.h"

namespace PythonScript
{
    //the wrapper can't directly keep these 
    static std::map<Core::entity_id_t, Scene::EntityPtr> entity_ptrs; //XXX should definitely be weakrefs (right?)

    typedef struct {
        PyObject_HEAD
        /* Type-specific fields go here. */
        //Scene::EntityPtr entity;
        //smart_ptrs can't be just like this in pyobjects, see e.g. http://wiki.python.org/moin/boost.python/PointersAndSmartPointers 
        Core::entity_id_t ent_id;
    } rexviewer_EntityObject;

    //why can't these be static? doesn't find the definitions in that case..
    void entity_init(PyObject* m);
    PyObject* entity_create(Core::entity_id_t ent_id); //, Scene::EntityPtr entity);
    PyObject* entity_getattro(PyObject *self, PyObject *name);
    int entity_setattro(PyObject *self, PyObject *name, PyObject *value);

    static PyTypeObject rexviewer_EntityType = {
        PyObject_HEAD_INIT(NULL)
        0,                         /*ob_size*/
        "rexviewer.Entity",             /*tp_name*/
        sizeof(rexviewer_EntityObject), /*tp_basicsize*/
        0,                         /*tp_itemsize*/
        0,                         /*tp_dealloc*/
        0,                         /*tp_print*/
        0,                         /*tp_getattr*/
        0,                         /*tp_setattr*/
        0,                         /*tp_compare*/
        0,                         /*tp_repr*/
        0,                         /*tp_as_number*/
        0,                         /*tp_as_sequence*/
        0,                         /*tp_as_mapping*/
        0,                         /*tp_hash */
        0,                         /*tp_call*/
        0,                         /*tp_str*/
        entity_getattro,           /*tp_getattro*/
        entity_setattro,          /*tp_setattro*/
        0,                         /*tp_as_buffer*/
        Py_TPFLAGS_DEFAULT,        /*tp_flags*/
        "Entity object",           /* tp_doc */
    };
}

#endif
#include "StableHeaders.h"

#include "PythonScriptModule.h" //for the staticframework hack, for entityptr re-get workaround
#include "Entity.h"

#include "RexLogicModule.h" //for getting prim component data
#include "EC_OpenSimPrim.h"
#include "EC_OgrePlaceable.h"

/* this is not a module but defines a type for the module created in PythonScriptModule */
//initentity(void) 

namespace PythonScript
{
	static PyMethodDef entity_methods[] = {
		{NULL}  /* Sentinel */
	};

	void PythonScript::entity_init(PyObject* m)
	{
 		rexviewer_EntityType.tp_new = PyType_GenericNew;
		if (PyType_Ready(&rexviewer_EntityType) < 0)
		{
			std::cout << "PythonScriptModule: EntityType not ready?" << std::endl;
			return;
		}

		Py_INCREF(&rexviewer_EntityType);
		PyModule_AddObject(m, "Entity", (PyObject *)&rexviewer_EntityType);
	}

	PyObject* PythonScript::entity_create(Core::entity_id_t ent_id) //, Foundation::EntityPtr entity)
	{
		rexviewer_EntityObject* eob;
		std::cout << "Entity: creating a wrapper pyobject ..";
		eob = PyObject_New(rexviewer_EntityObject, &rexviewer_EntityType); //sets refcount to 1

		//std::cout << "setting the pointer to the entity in the wrapper: " << entity << std::endl;
		//eob->entity = entity; //doesn't have a constructor, just this factory
		
		//std::cout << "storing the pointer to the entity in the entity_ptrs map:" << entity << std::endl;
		//entity_ptrs[ent_id] = entity;

		std::cout << "storing the entity id in the wrapper object:" << ent_id << std::endl;
		eob->ent_id = ent_id;
		return (PyObject*) eob;
	}

	PyObject* PythonScript::entity_getattro(PythonScript::rexviewer_EntityObject *self, PyObject *name)
	{
		PyObject* tmp;

		if (!(tmp = PyObject_GenericGetAttr((PyObject*)self, name))) {
			if (!PyErr_ExceptionMatches(PyExc_AttributeError))
				return NULL;
			PyErr_Clear();
		}
		else
			return tmp;

		char* s_name = PyString_AsString(name);

		std::cout << "Entity: getting unknown attribute: " << s_name;
		
		//entity_ptrs map usage
		/* this crashes now in boost, 
		   void add_ref_copy() { BOOST_INTERLOCKED_INCREMENT( &use_count_ );
		std::map<Core::entity_id_t, Foundation::EntityPtr>::iterator ep_iter = entity_ptrs.find(self->ent_id);
		Foundation::EntityPtr entity = ep_iter->second;
		fix.. */

		/* re-getting the EntityPtr as it wasn't stored anywhere yet,
		   is copy-paste from PythonScriptModule GetEntity 
		   but to be removed when that map is used above.*/
		//Foundation::Framework *framework_ = PythonScript::staticframework;
		//XXX crashes 'cause this is not static. why is this definition not found when is made static?
		static Foundation::Framework *framework_ = PythonScript::staticframework;
		Foundation::SceneManagerServiceInterface *sceneManagerService = framework_->GetService<Foundation::SceneManagerServiceInterface>(Foundation::Service::ST_SceneManager);
		const Foundation::ScenePtr &scene = sceneManagerService->GetScene("World"); //XXX hardcoded scene name, like in debugstats now
		if (scene == 0)
			return NULL; //XXX return some sensible exception info

		std::cout << ".. regetting the EntityPtr to entity id " << self->ent_id;
		const Foundation::EntityPtr entity = scene->GetEntity(self->ent_id);
		/* end copy-paste / ptr re-get */

		if (s_name == "prim")
		{
			std::cout << ".. getting prim" << std::endl;
			const Foundation::ComponentInterfacePtr &prim_component = entity->GetComponent("EC_OpenSimPrim");
			if (!prim_component)
				return NULL; //XXX report AttributeError
			RexLogic::EC_OpenSimPrim *prim = checked_static_cast<RexLogic::EC_OpenSimPrim *>(prim_component.get());
		        
			//m->AddU32(prim->LocalId);
			std::string retstr = "local id:" + prim->FullId.ToString() + "- prim name: " + prim->ObjectName;
			return PyString_FromString(retstr.c_str());
		}

		else if (s_name == "place")
		{
	        const Foundation::ComponentInterfacePtr &ogre_component = entity->GetComponent("EC_OgrePlaceable");
			if (!ogre_component)
	            return NULL; //XXX report AttributeError        
			OgreRenderer::EC_OgrePlaceable *placeable = checked_static_cast<OgreRenderer::EC_OgrePlaceable *>(ogre_component.get());
			
			/* this must probably return a new object, a 'Place' instance, that has these.
			   or do we wanna hide the E-C system in the api and have these directly on entity? 
			   probably not a good idea to hide the actual system that much. or? */
			RexTypes::Vector3 pos = placeable->GetPosition();
			//RexTypes::Vector3 scale = ogre_pos->GetScale();
			//RexTypes::Vector3 rot = Core::PackQuaternionToFloat3(ogre_pos->GetOrientation());
			/* .. i guess best to wrap the Rex Vector and other types soon,
			   the pyrr irrlicht binding project does it for these using swig,
			   https://opensvn.csie.org/traccgi/pyrr/browser/pyrr/irrlicht.i */
			return Py_BuildValue("fff", pos.x, pos.y, pos.z);
		}

		std::cout << "unknown component type."  << std::endl;
		return NULL;
	}

	void foo()
	{
		int a = 1 + 1;
	}
}
// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_SceneManagerInterface_h
#define incl_Interfaces_SceneManagerInterface_h

#include <boost/shared_ptr.hpp>
#include "EntityInterface.h"

namespace Scene
{
    class SceneManagerInterface;
    typedef boost::shared_ptr<SceneManagerInterface> ScenePtr;
    typedef boost::weak_ptr<SceneManagerInterface> SceneWeakPtr;

    //! Acts as a generic scenegraph for all entities in the scene.
    /*!
        Contains all entities in the scene in a generic fashion.
        Acts as a factory for all entities.

        \ingroup Scene_group
    */
    class SceneManagerInterface
    {
    public:
        //! default constructor
        SceneManagerInterface() {}

        //! destructor
        virtual ~SceneManagerInterface() {}

        //! assignment operator
        virtual SceneManagerInterface &operator =(const SceneManagerInterface &other) = 0;
        //! Returns true if the two scenes have the same name
        virtual bool operator == (const SceneManagerInterface &other) const = 0;
        //! Returns true if the two scenes have different names
        virtual bool operator != (const SceneManagerInterface &other) const = 0;
        //! Order by name
        virtual bool operator < (const SceneManagerInterface &other) const = 0;

        //! Returns scene name
        virtual const std::string &Name() const = 0;

        //! Make a soft clone of this scene. The new scene will contain the same entities as the old one.
        /*! 
            \param newName Name of the new scene
        */
        virtual ScenePtr Clone(const std::string &newName) const = 0;

        //! Creates new entity that contains the specified components
        /*! Entities should never be created directly, but instead created with this function.

            To create an empty entity omit components parameter.

            \param id Id of the new entity. Use GetNextFreeId().
            \param components Optional list of component names the entity will use. If omitted or the list is empty, creates an empty entity.
        */
        virtual EntityPtr CreateEntity(Core::entity_id_t id = 0, const Core::StringVector &components = Core::StringVector()) = 0;

        //! Makes a soft clone of the entity. The new entity will be placed in this scene.
        /*! The entity need not be contained in this scene

            \param entity Entity to be cloned
        */
        virtual EntityPtr CloneEntity(const EntityPtr &entity) = 0;
        
        //! Returns entity with the specified id
        /*!
            \note Returns a shared pointer, but it is preferable to use a weak pointer, Scene::EntityWeakPtr,
                  to avoid dangling references that prevent entities from being properly destroyed.
        */
        virtual Scene::EntityPtr GetEntity(Core::entity_id_t id) const = 0;

        //! Returns true if entity with the specified id exists in this scene, false otherwise
        virtual bool HasEntity(Core::entity_id_t id) const { return GetEntity(id).get() != 0; }
        
        //! Remove entity with specified id
        /*! The entity may not get deleted if the entity is shared between multiple scenes,
            or if dangling references to a pointer to the entity exists.

            \param id Id of the entity to remove
        */
        virtual void RemoveEntity(Core::entity_id_t id) = 0;       
        
        //! Get the next free entity id. Can be used with CreateEntity().
        virtual Core::entity_id_t GetNextFreeId() = 0;

    public:
        //! Virtual scene entity iterator interface. When implemeting SceneInterface, one must provide the
        //! following functionality as well. This iterator only supports forward sequential access. Any call to
        //! a nonconst method of SceneInterface may invalidate any existing iterators.
        class EntityIteratorInterface
        {
        public:
            virtual ~EntityIteratorInterface() {}

            virtual bool operator !=(const EntityIteratorInterface &rhs) const = 0;

            virtual EntityIteratorInterface &operator ++() = 0;

            virtual EntityInterface &operator *() = 0;
        };

        class ConstEntityIteratorInterface
        {
        public:
            virtual ~ConstEntityIteratorInterface() {}

            virtual bool operator !=(const ConstEntityIteratorInterface &rhs) const = 0;

            virtual ConstEntityIteratorInterface &operator ++() = 0;

            virtual const EntityInterface &operator *() = 0;
        };

        typedef boost::shared_ptr<EntityIteratorInterface> SceneIteratorPtr;
        typedef boost::shared_ptr<ConstEntityIteratorInterface> ConstSceneIteratorPtr;

        virtual SceneIteratorPtr SceneIteratorBegin() = 0;
        virtual SceneIteratorPtr SceneIteratorEnd() = 0;

        virtual ConstSceneIteratorPtr SceneIteratorBegin() const = 0;
        virtual ConstSceneIteratorPtr SceneIteratorEnd() const = 0;

    public:
        class EntityIterator
        {
        public:
            EntityIterator(SceneIteratorPtr impl):impl_(impl) {}
            ~EntityIterator() {}

            bool operator !=(const EntityIterator &rhs) const { return *impl_ != *rhs.impl_; }

            EntityIterator &operator ++() { ++(*impl_); return *this; }

            Scene::EntityInterface &operator *() { assert(impl_); return **impl_; }
        private:
            SceneIteratorPtr impl_;
        };

        class MODULE_API ConstEntityIterator
        {
        public:
            ConstEntityIterator(ConstSceneIteratorPtr impl):impl_(impl) {}
            ~ConstEntityIterator() {}

            bool operator !=(const ConstEntityIterator &rhs) const { return *impl_ != *rhs.impl_; }

            ConstEntityIterator &operator ++() { ++(*impl_); return *this; }

            const Scene::EntityInterface &operator *() { assert(impl_); return **impl_; }
        private:
            ConstSceneIteratorPtr impl_;
        };

        EntityIterator begin() { return EntityIterator(SceneIteratorBegin()); }
        EntityIterator end() { return EntityIterator(SceneIteratorEnd()); }

        ConstEntityIterator begin() const { return ConstEntityIterator(SceneIteratorBegin()); }
        ConstEntityIterator end() const { return ConstEntityIterator(SceneIteratorEnd()); }

    };
}

#endif


// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_SceneInterface_h
#define incl_Interfaces_SceneInterface_h

#include <boost/shared_ptr.hpp>
#include "EntityInterface.h"

namespace Foundation
{
    class SceneInterface;
    typedef boost::shared_ptr<SceneInterface> ScenePtr;

    //! Acts as a generic scenegraph for all entities in the world
    /*!
        Contains all entities in the world in a generic fashion.
        Acts as a factory for all entities.
    */
    class MODULE_API SceneInterface
    {
    public:
        //! default constructor
        SceneInterface() {}

        //! destructor
        virtual ~SceneInterface() {}

        virtual bool operator == (const SceneInterface &other) const = 0;
        virtual bool operator != (const SceneInterface &other) const = 0;
        virtual bool operator < (const SceneInterface &other) const = 0;

        //! Returns scene name
        virtual const std::string &Name() const = 0;

        //! Make a soft clone of this scene. The new scene will contain the same entities as the old one.
        /*! 
            \param newName Name of the new scene
        */
        virtual ScenePtr Clone(const std::string &newName) const = 0;

        //! Creates new entity that contains the specified components
        /*!
            \param components list of component names the entity will use
        */
        virtual EntityPtr CreateEntity(Core::entity_id_t id, const Core::StringVector &components) = 0;

        //! Creates an empty entity
        virtual Foundation::EntityPtr CreateEntity(Core::entity_id_t id) = 0;

        //! Makes a soft clone of the entity. The new entity will be placed in this scene.
        /*! The entity need not be contained in this scene

            \param entity Entity to be cloned
        */
        virtual Foundation::EntityPtr CloneEntity(const EntityPtr &entity) = 0;
        
        //! Returns entity with the specified id
        virtual Foundation::EntityPtr GetEntity(Core::entity_id_t id) const = 0;

        //! Returns true if entity with the specified id exists in this scene, false otherwise
        virtual bool HasEntity(Core::entity_id_t id) const { return GetEntity(id).get() != 0; }
        
        //! Get the next free id
        virtual Core::entity_id_t GetNextFreeId() = 0;

    public:
        //! Virtual scene entity iterator interface. When implemeting SceneInterface, one must provide the
        //! following functionality as well. This iterator only supports forward sequential access. Any call to
        //! a nonconst method of SceneInterface may invalidate any existing iterators.
        class MODULE_API EntityIteratorInterface
        {
        public:
            virtual ~EntityIteratorInterface() {}

            virtual bool operator !=(const EntityIteratorInterface &rhs) const = 0;

            virtual EntityIteratorInterface &operator ++() = 0;

            virtual EntityInterface &operator *() = 0;
        };

        class MODULE_API ConstEntityIteratorInterface
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
        class MODULE_API EntityIterator
        {
        public:
            EntityIterator(SceneIteratorPtr impl):impl_(impl) {}
            ~EntityIterator() {}

            bool operator !=(const EntityIterator &rhs) const { return *impl_ != *rhs.impl_; }

            EntityIterator &operator ++() { ++(*impl_); return *this; }

            Foundation::EntityInterface &operator *() { assert(impl_); return **impl_; }
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

            const Foundation::EntityInterface &operator *() { assert(impl_); return **impl_; }
        private:
            ConstSceneIteratorPtr impl_;
        };

        EntityIterator begin() { return EntityIterator(SceneIteratorBegin()); }
        EntityIterator end() { return EntityIterator(SceneIteratorEnd()); }

        ConstEntityIterator begin() const { return ConstEntityIterator(SceneIteratorBegin()); }
        ConstEntityIterator end() const { return ConstEntityIterator(SceneIteratorEnd()); }

    };

    //! Acts as a generic scenegraph for all entities in the world
    /*!
        Contains all entities in the world in a generic fashion.
        Acts as a factory for all entities.
    */
    class MODULE_API SceneInterfaceImpl : public SceneInterface
    {
    public:
        //! default constructor
        SceneInterfaceImpl() {}

        //! constructor that takes a string as name
        SceneInterfaceImpl(const std::string &name) : name_(name) {}

        //! copy constructor
        SceneInterfaceImpl( const SceneInterfaceImpl &other ) : SceneInterface(), name_(other.name_) { }

        //! destructor
        virtual ~SceneInterfaceImpl() {}
        
        //! Returns scene name
        virtual const std::string &Name() const { return name_; }

    private:
        //! name of the module
        const std::string name_;
    };
}

#endif


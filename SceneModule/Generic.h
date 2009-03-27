// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_SceneGeneric_h
#define incl_SceneGeneric_h

#include "SceneInterface.h"

namespace Scene
{
    class SceneModule;

    //! Acts as a generic scenegraph for all entities in the world
    /*!
        Contains all entities in the world in a generic fashion.
        Acts as a factory for all entities.
    */
    class MODULE_API Generic : public Foundation::SceneInterface_Impl
    {
        friend class SceneManager;
    private:
        Generic();
        //! constructor that takes a framework
        Generic(const std::string &name, SceneModule *module) : SceneInterface_Impl(name), module_(module) {}
        //! copy constructor that also takes a name
        Generic( const Generic &other, const std::string &name ) : SceneInterface_Impl(name), module_(other.module_), entities_(other.entities_) { }
        // copy constuctor
        Generic( const Generic &other);

        typedef std::map<Core::entity_id_t, Foundation::EntityPtr> EntityMap;

        //! Current global id for entities
        static Core::uint gid_;

    public:
        virtual ~Generic() {}

        const Generic &operator =(const Generic &other)
        {
            if (&other != this)
            {
                entities_ = other.entities_;
            }
            return *this;
        }

        virtual bool operator == (const SceneInterface &other) const { return Name() == other.Name(); }
        virtual bool operator != (const SceneInterface &other) const { return !(*this == other); }
        virtual bool operator < (const SceneInterface &other) const { return Name() < other.Name(); }

        //! Make a soft clone of this scene. The new scene will contain the same entities as the old one.
        /*! 
            \param newName Name of the new scene
        */
        virtual Foundation::ScenePtr Clone(const std::string &newName) const;

        //! Creates new entity that contains the specified components
        /*!
            \param components list of component names the entity will use
        */
        virtual Foundation::EntityPtr CreateEntity(Core::entity_id_t id, const Core::StringVector &components);

        //! Creates an empty entity
        virtual Foundation::EntityPtr CreateEntity(Core::entity_id_t id);

        //! Makes a soft clone of the entity. The new entity will be placed in this scene.
        /*! The entity need not be contained in this scene

            \param entity Entity to be cloned
        */
        virtual Foundation::EntityPtr CloneEntity(const Foundation::EntityPtr &entity);
        
        //! Returns entity with the specified id
        virtual Foundation::EntityPtr GetEntity(Core::entity_id_t id) const;

        virtual bool HasEntity(Core::entity_id_t id) const
        {
            return (entities_.find(id) != entities_.end());
        }

        class MODULE_API EntityIteratorImpl : public Foundation::SceneInterface::EntityIteratorInterface
        {
        public:
            EntityIteratorImpl(EntityMap::iterator iter):iter_(iter) {}
            ~EntityIteratorImpl() {}

            bool operator !=(const Foundation::SceneInterface::EntityIteratorInterface &rhs) const { return this->iter_ != dynamic_cast<const EntityIteratorImpl&>(rhs).iter_; }

            SceneInterface::EntityIteratorInterface &operator ++() { ++iter_; return *this; }

            Foundation::EntityInterface &operator *() { return *iter_->second; }

        private:
            EntityMap::iterator iter_;
        };

        SceneIteratorPtr SceneIteratorBegin() { return SceneIteratorPtr(new EntityIteratorImpl(entities_.begin())); }
        SceneIteratorPtr SceneIteratorEnd() { return SceneIteratorPtr(new EntityIteratorImpl(entities_.end())); }

        class MODULE_API ConstEntityIteratorImpl : public SceneInterface::ConstEntityIteratorInterface
        {
        public:
            ConstEntityIteratorImpl(EntityMap::const_iterator iter):iter_(iter) {}
            ~ConstEntityIteratorImpl() {}

            bool operator !=(const SceneInterface::ConstEntityIteratorInterface &rhs) const { return this->iter_ != dynamic_cast<const ConstEntityIteratorImpl&>(rhs).iter_; }

            SceneInterface::ConstEntityIteratorInterface &operator ++() { ++iter_; return *this; }

            const Foundation::EntityInterface &operator *() { return *iter_->second; }

        private:
            EntityMap::const_iterator iter_;
        };

        ConstSceneIteratorPtr SceneIteratorBegin() const { return ConstSceneIteratorPtr(new ConstEntityIteratorImpl(entities_.begin())); }
        ConstSceneIteratorPtr SceneIteratorEnd() const { return ConstSceneIteratorPtr(new ConstEntityIteratorImpl(entities_.end())); }

        //! Returns next available id
        virtual Core::entity_id_t GetNextFreeId();
    
    private:

        //! Entities in a map
        EntityMap entities_;

        //! parent module
        SceneModule *module_;
    };
}

#endif

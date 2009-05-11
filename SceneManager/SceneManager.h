// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_SceneSceneManager_h
#define incl_SceneSceneManager_h

#include "SceneManagerInterface.h"

namespace Scene
{
    //! Acts as a generic scenegraph for all entities in the world
    /*!
        Contains all entities in the world in a generic fashion.
        Acts as a factory for all entities.
    */
    class SceneManager : public Scene::SceneManagerInterface
    {
        friend class Foundation::Framework;
    private:
        SceneManager();
        //! constructor that takes a name and parent module
        SceneManager(const std::string &name, Foundation::Framework *framework) : SceneManagerInterface(), name_(name), framework_(framework) {}
        //! copy constructor that also takes a name
        SceneManager( const SceneManager &other, const std::string &name ) : SceneManagerInterface(), framework_(other.framework_), entities_(other.entities_) { }
        // copy constuctor
        SceneManager( const SceneManager &other);

        typedef std::map<Core::entity_id_t, Scene::EntityPtr> EntityMap;

        //! Current global id for entities
        static Core::uint gid_;

    public:
        virtual ~SceneManager() {}

        SceneManagerInterface &operator =(const SceneManagerInterface &other)
        {
            if (&other != this)
            {
                entities_ = checked_static_cast<const SceneManager*>(&other)->entities_;
            }
            return *this;
        }

        bool operator == (const SceneManagerInterface &other) const { return Name() == other.Name(); }
        bool operator != (const SceneManagerInterface &other) const { return !(*this == other); }
        bool operator <  (const SceneManagerInterface &other) const { return Name() < other.Name(); }

        const std::string &Name() const { return name_; }

        Scene::ScenePtr Clone(const std::string &newName) const;
        Scene::EntityPtr CreateEntity(Core::entity_id_t id = 0, const Core::StringVector &components = Core::StringVector());
        Scene::EntityPtr CloneEntity(const Scene::EntityPtr &entity);
        Scene::EntityPtr GetEntity(Core::entity_id_t id) const;

        bool HasEntity(Core::entity_id_t id) const
        {
            return (entities_.find(id) != entities_.end());
        }

        void RemoveEntity(Core::entity_id_t id);

        Core::entity_id_t GetNextFreeId();


        /// Implements a non-const sequential iterator for accessing the entities in the scene.
        class EntityIteratorImpl : public Scene::SceneManagerInterface::EntityIteratorInterface
        {
        public:
            EntityIteratorImpl(EntityMap::iterator iter):iter_(iter) {}
            ~EntityIteratorImpl() {}

            bool operator !=(const Scene::SceneManagerInterface::EntityIteratorInterface &rhs) const { return this->iter_ != dynamic_cast<const EntityIteratorImpl&>(rhs).iter_; }

            SceneManagerInterface::EntityIteratorInterface &operator ++() { ++iter_; return *this; }

            Scene::EntityInterface &operator *() { return *iter_->second; }

        private:
            EntityMap::iterator iter_;
        };

        SceneIteratorPtr SceneIteratorBegin() { return SceneIteratorPtr(new EntityIteratorImpl(entities_.begin())); }
        SceneIteratorPtr SceneIteratorEnd() { return SceneIteratorPtr(new EntityIteratorImpl(entities_.end())); }

        /// Implements a const sequential iterator for accessing the entities in the scene.
        class ConstEntityIteratorImpl : public SceneManagerInterface::ConstEntityIteratorInterface
        {
        public:
            ConstEntityIteratorImpl(EntityMap::const_iterator iter):iter_(iter) {}
            ~ConstEntityIteratorImpl() {}

            bool operator !=(const SceneManagerInterface::ConstEntityIteratorInterface &rhs) const { return this->iter_ != dynamic_cast<const ConstEntityIteratorImpl&>(rhs).iter_; }

            SceneManagerInterface::ConstEntityIteratorInterface &operator ++() { ++iter_; return *this; }

            const Scene::EntityInterface &operator *() { return *iter_->second; }

        private:
            EntityMap::const_iterator iter_;
        };

        ConstSceneIteratorPtr SceneIteratorBegin() const { return ConstSceneIteratorPtr(new ConstEntityIteratorImpl(entities_.begin())); }
        ConstSceneIteratorPtr SceneIteratorEnd() const { return ConstSceneIteratorPtr(new ConstEntityIteratorImpl(entities_.end())); }
    
    private:

        //! Entities in a map
        EntityMap entities_;

        Foundation::Framework *framework_;

        //! Name of the scene
        const std::string name_;
    };
}

#endif

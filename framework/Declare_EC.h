// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Foundation_EC_Define_h
#define incl_Foundation_EC_Define_h

#define DECLARE_EC(component)                                                               \
    public:                                                                                 \
    class component##Registrar : public Foundation::ComponentRegistrarInterface {           \
    public:                                                                                 \
        component##Registrar() {}                                                           \
        virtual ~component##Registrar() {}                                                  \
        virtual void _register(Foundation::Framework *framework) {                          \
            component::registerComponent(framework);                                        \
        }                                                                                   \
        virtual void _unregister(Foundation::Framework *framework) {                        \
            component::unregisterComponent(framework);                                      \
        }                                                                                   \
    };                                                                                      \
    private:                                                                                \
    class component##Factory : public Foundation::ComponentFactoryInterface {               \
    public:                                                                                 \
        component##Factory() {}                                                             \
        virtual ~component##Factory() {}                                                    \
                                                                                            \
        virtual Foundation::ComponentInterfacePtr  operator()() {                           \
            return Foundation::ComponentInterfacePtr(new component);                        \
        }                                                                                   \
        virtual Foundation::ComponentInterfacePtr  operator()(                              \
                const Foundation::ComponentInterfacePtr &other) {                           \
           return Foundation::ComponentInterfacePtr(                                        \
                  new component(*static_cast<component*>(other.get())));                    \
        }                                                                                   \
    };                                                                                      \
    friend class component##Factory;                                                        \
    public:                                                                                 \
    static void registerComponent(const Foundation::Framework *framework)                   \
    {                                                                                       \
        Foundation::ComponentFactoryInterfacePtr factory =                                  \
            Foundation::ComponentFactoryInterfacePtr(new component##Factory);               \
        framework->getComponentManager()->registerFactory(name(), factory);                 \
    }                                                                                       \
    static void unregisterComponent(const Foundation::Framework *framework) {               \
        framework->getComponentManager()->unregisterFactory(name());                        \
    }                                                                                       \
    static const std::string &name()                                                        \
    {                                                                                       \
        static const std::string name(#component);                                          \
        return name;                                                                        \
    }                                                                                       \
    virtual const std::string &_name() {                                                    \
        return component::name();                                                           \
    }                                                                                       \
    private:                                                                                \


#endif

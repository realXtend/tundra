// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Foundation_EC_Define_h
#define incl_Foundation_EC_Define_h

#define DECLARE_EC(component)                                                               \
    public:                                                                                 \
    class component##Registrar : public Foundation::ComponentRegistrarInterface {           \
    public:                                                                                 \
        component##Registrar() {}                                                           \
        virtual ~component##Registrar() {}                                                  \
        virtual void Register(Foundation::Framework *framework) {                           \
            component::RegisterComponent(framework);                                        \
        }                                                                                   \
        virtual void Unregister(Foundation::Framework *framework) {                         \
            component::UnregisterComponent(framework);                                      \
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
    static void RegisterComponent(const Foundation::Framework *framework)                   \
    {                                                                                       \
        Foundation::ComponentFactoryInterfacePtr factory =                                  \
            Foundation::ComponentFactoryInterfacePtr(new component##Factory);               \
        framework->GetComponentManager()->RegisterFactory(Name(), factory);                 \
    }                                                                                       \
    static void UnregisterComponent(const Foundation::Framework *framework) {               \
        framework->GetComponentManager()->UnregisterFactory(Name());                        \
    }                                                                                       \
    static const std::string &Name()                                                        \
    {                                                                                       \
        static const std::string name(#component);                                          \
        return name;                                                                        \
    }                                                                                       \
    virtual const std::string &_Name() {                                                    \
        return component::Name();                                                           \
    }                                                                                       \
    private:                                                                                \


#endif

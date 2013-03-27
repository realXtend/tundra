// For conditions of distribution and use, see copyright notice in license.txt

template <typename T>
shared_ptr<T> Entity::CreateComponent(const QString &name, AttributeChange::Type change, bool replicated)
{
    return dynamic_pointer_cast<T>(CreateComponent(T::ComponentTypeId, name, change, replicated)); /**< @todo static_pointer_cast should be ok here. */
}

template<typename T>
shared_ptr<T> Entity::GetOrCreateComponent(const QString &name, AttributeChange::Type change, bool replicated)
{
    shared_ptr<T> existing = name.isEmpty() ? Component<T>() : Component<T>(name);
    if (existing)
        return existing;

    return CreateComponent<T>(name, change, replicated);
}

template <class T>
shared_ptr<T> Entity::Component() const
{
    return dynamic_pointer_cast<T>(Component(T::TypeIdStatic())); /**< @todo static_pointer_cast should be ok here. */
}

template <class T>
std::vector<shared_ptr<T> > Entity::ComponentsOfType() const
{
    std::vector<shared_ptr<T> > ret;
    for(ComponentMap::const_iterator i = components_.begin(); i != components_.end(); ++i)
    {
        shared_ptr<T> t = dynamic_pointer_cast<T>(i->second); /**< @todo static_pointer_cast should be ok here. */
        if (t)
            ret.push_back(t);
    }
    return ret;
}

template <class T>
shared_ptr<T> Entity::Component(const QString& name) const
{
    return dynamic_pointer_cast<T>(Component(T::TypeIdStatic(), name));
}

template<typename T>
Attribute<T> *Entity::GetAttribute(const QString &name) const
{
    for(ComponentMap::const_iterator i = components_.begin(); i != components_.end(); ++i)
    {
        Attribute<T> *t = i->second->GetAttribute<T>(name);
        if (t)
            return t;
    }
    return 0;
}

template<typename T>
std::vector<Attribute<T> > Entity::GetAttributes(const QString &name) const
{
    std::vector<Attribute<T> > ret;
    for(ComponentMap::const_iterator i = components_.begin(); i != components_.end(); ++i)
    {
        Attribute<T> *t = i->second->GetAttribute<T>(name);
        if (t)
            return ret.push_back(t);
    }
    return ret;
}

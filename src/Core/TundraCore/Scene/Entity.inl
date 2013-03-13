// For conditions of distribution and use, see copyright notice in license.txt

template <class T>
shared_ptr<T> Entity::GetComponent() const
{
    return dynamic_pointer_cast<T>(GetComponent(T::TypeNameStatic()));
}

template <class T>
std::vector<shared_ptr<T> > Entity::GetComponents() const
{
    std::vector<shared_ptr<T> > ret;
    for (ComponentMap::const_iterator i = components_.begin(); i != components_.end(); ++i)
    {
        shared_ptr<T> t = dynamic_pointer_cast<T>(i->second);
        if (t)
            ret.push_back(t);
    }
    return ret;
}

template <class T>
shared_ptr<T> Entity::GetComponent(const QString& name) const
{
    return dynamic_pointer_cast<T>(GetComponent(T::TypeNameStatic(), name));
}

template<typename T>
Attribute<T> *Entity::GetAttribute(const QString &name) const
{
    for (ComponentMap::const_iterator i = components_.begin(); i != components_.end(); ++i)
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
    for (ComponentMap::const_iterator i = components_.begin(); i != components_.end(); ++i)
    {
        Attribute<T> *t = i->second->GetAttribute<T>(name);
        if (t)
            return ret.push_back(t);
    }
    return ret;
}

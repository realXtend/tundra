// For conditions of distribution and use, see copyright notice in license.txt

template <class T>
boost::shared_ptr<T> Entity::GetComponent() const
{
    return boost::dynamic_pointer_cast<T>(GetComponent(T::TypeNameStatic()));
}

template <class T>
std::vector<boost::shared_ptr<T> > Entity::GetComponents() const
{
    std::vector<boost::shared_ptr<T> > ret;
    for(size_t i = 0; i < components_.size() ; ++i)
    {
        boost::shared_ptr<T> t = boost::dynamic_pointer_cast<T>(components_[i]);
        if (t)
            ret.push_back(t);
    }
    return ret;
}

template <class T>
boost::shared_ptr<T> Entity::GetComponent(const QString& name) const
{
    return boost::dynamic_pointer_cast<T>(GetComponent(T::TypeNameStatic(), name));
}

template<typename T>
Attribute<T> *Entity::GetAttribute(const std::string &name) const
{
    for(size_t i = 0; i < components_.size() ; ++i)
    {
        Attribute<T> *t = components_[i]->GetAttribute<T>(name);
        if (t)
            return t;
    }
    return 0;
}

template<typename T>
std::vector<Attribute<T> > Entity::GetAttributes(const std::string &name) const
{
    std::vector<Attribute<T> > ret;
    for(size_t i = 0; i < components_.size() ; ++i)
    {
        Attribute<T> *t = components_[i]->GetAttribute<T>(name);
        if (t)
            return ret.push_back(t);
    }
    return ret;
}

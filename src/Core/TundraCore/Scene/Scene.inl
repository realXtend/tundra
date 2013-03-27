// For conditions of distribution and use, see copyright notice in license.txt

template <class T>
shared_ptr<T> Scene::GetWorld() const
{
    QVariant pr = this->property(T::PropertyName());
    QObject *qo = pr.value<QObject*>();
    T* rawPtr = checked_static_cast<T*>(qo);
    return rawPtr ? rawPtr->shared_from_this() : shared_ptr<T>();
}

template <typename T>
std::vector<shared_ptr<T> > Scene::Components(const QString &name) const
{
    std::vector<shared_ptr<T> > ret;
    if (name.isEmpty())
    {
        for(const_iterator it = begin(); it != end(); ++it)
        {
            std::vector<shared_ptr<T> > components =  it->second->ComponentsOfType<T>();
            if (!components.empty())
                ret.insert(ret.end(), components.begin(), components.end());
        }
    }
    else
    {
        for(const_iterator it = begin(); it != end(); ++it)
        {
            shared_ptr<T> component = it->second->GetComponent<T>(name);
            if (component)
                ret.push_back(component);
        }
    }
    return ret;
}

template <typename T>
EntityList Scene::EntitiesWithComponent(const QString &name) const
{
    return EntitiesWithComponent(T::ComponentTypeId, name);
}

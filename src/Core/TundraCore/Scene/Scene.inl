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
    std::vector<shared_ptr<T> > components;
    for(const_iterator it = begin(); it != end(); ++it)
    {
        shared_ptr<T> component = (name.isEmpty() ? it->second->GetComponent<T>() : it->second->GetComponent<T>(name));
        if (component)
            components.push_back(component);
    }
    return components;
}

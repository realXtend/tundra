// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

template<typename T>
void ECAttributeEditor<T>::SetValue(const T &value)
{
    foreach(const ComponentWeakPtr &comp, components_)
        if (!comp.expired())
            SetValue(comp.lock(), value);
}

template<typename T>
void ECAttributeEditor<T>::SetValue(const ComponentPtr &comp, const T &value)
{
    Attribute<T> *attr = FindAttribute<T>(comp);
    if(attr)
    {
        listenEditorChangedSignal_ = false;
        emit AttributeAboutToBeEdited(attr);
        attr->Set(value, AttributeChange::Default);
        listenEditorChangedSignal_ = true;
    }
}

template<typename T>
bool ECAttributeEditor<T>::HasIdenticalAttributes() const
{
    //No point to continue if there is only single component added.
    if(components_.size() <= 1)
        return true;

    ComponentWeakPtrList::const_iterator lsh = components_.begin();
    IComponent *lsh_comp = dynamic_cast<IComponent*>((*lsh).lock().get());
    if(!lsh_comp)
        return false;
    Attribute<T> *lsh_attr = dynamic_cast<Attribute<T>*>(lsh_comp->GetAttribute(name_));

    IComponent *rsh_comp = 0;
    Attribute<T> *rsh_attr = 0;
    ComponentWeakPtrList::const_iterator rhs = ++components_.begin();
    for(;rhs != components_.end(); rhs++)
    {
        rsh_comp = (*rhs).lock().get();
        if (!rsh_comp) //Ensure that component hasn't expired.
            continue;
        rsh_attr = dynamic_cast<Attribute<T>*>(rsh_comp->GetAttribute(name_));
        if (!rsh_attr)
            continue;

        if (lsh_attr->Get() != rsh_attr->Get())
            return false;
    }
    return true;
}

template<typename T>
void ECAttributeEditor<T>::UpdateMultiEditorValue(IAttribute *attribute)
{
    QStringList stringList;
    MultiEditPropertyManager *propertyManager = dynamic_cast<MultiEditPropertyManager *>(propertyMgr_);
    if (!propertyManager) // If editor's ui isn't initialized no point to continue.
        return;

    if(attribute)
    {
        QStringList values = propertyManager->AttributeValue(rootProperty_);
        QString value = QString::fromStdString(attribute->ToString());
        if (!values.contains(value))
        {
            values.push_back(value);
            propertyManager->SetAttributeValues(rootProperty_, stringList);
        }
    }
    else // This should only get called only once when attribute editor has just switched to multieditor mode.
    {
        foreach(const ComponentWeakPtr &comp, components_)
        {
            Attribute<T> *attribute = dynamic_cast<Attribute<T>*>(FindAttribute(comp.lock()));
            if (!attribute)
            {
                /// @todo add log warning.
                continue;
            }

            QString newValue = QString::fromStdString(attribute->ToString());
            // Make sure that we wont insert same strings into the list.
            if(!stringList.contains(newValue))
                stringList << newValue;
        }

        propertyManager->SetAttributeValues(rootProperty_, stringList);
    }
}

template<typename T>
void ECAttributeEditor<T>::InitializeMultiEditor()
{
    ECAttributeEditorBase::PreInitialize();
    if(useMultiEditor_)
    {
        MultiEditPropertyManager *multiEditManager = new MultiEditPropertyManager(this);
        MultiEditPropertyFactory *multiEditFactory = new MultiEditPropertyFactory(this);
        
        propertyMgr_ = multiEditManager;
        factory_ = multiEditFactory;

        rootProperty_ = multiEditManager->addProperty(name_);
        owner_->setFactoryForManager(multiEditManager, multiEditFactory);
        UpdateMultiEditorValue();
        connect(multiEditManager, SIGNAL(ValueChanged(const QString &)), this, SLOT(MultiEditValueSelected(const QString &)));
    }
}

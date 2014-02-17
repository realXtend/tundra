// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "TundraCoreApi.h"
#include "IComponent.h"
#include "IAttribute.h"

/// Placeholder/fallback for components that don't currently have a registered C++ implementation.
class EC_PlaceholderComponent : public IComponent
{
public:
     /// @cond PRIVATE
    /// Do not directly allocate new components using operator new, but use the factory-based SceneAPI::CreateComponent functions instead.
    explicit EC_PlaceholderComponent(Scene* scene);
    /// @endcond
    ~EC_PlaceholderComponent();

    /// Returns the static typename of this component, which will always be CustomComponent.
    static const QString &TypeNameStatic()
    {
        static const QString name = EnsureTypeNameWithPrefix("CustomComponent");
        return name;
    }
    /// Returns the static type ID of this component. Always 0.
    static u32 TypeIdStatic()
    {
        return 0;
    }
    /// Returns the stored typename, ie. the typename this component is acting as a placeholder to.
    virtual const QString &TypeName() const
    {
        return typeName;
    }
    /// Returns the stored typeid, ie. the typeid this component is acting as a placeholder to.
    virtual u32 TypeId() const
    {
        return typeId;
    }

    /// IComponent override
    virtual void DeserializeFromBinary(kNet::DataDeserializer& source, AttributeChange::Type change);

    /// IComponent override
    /** PlaceholderComponent attributes need to be treated as static for the network protocol, though they are dynamically allocated */
    virtual int NumStaticAttributes() const { return attributes.size(); }

    void SetTypeId(u32 newTypeId);
    void SetTypeName(const QString& newTypeName);
    IAttribute *CreateAttribute(const QString &typeName, const QString &id, const QString &name, AttributeChange::Type change = AttributeChange::Default);

private:
    QString typeName;
    u32 typeId;
};

// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "AvatarDescAsset.h"
#include "AvatarModule.h"
#include "AssetAPI.h"
#include "Profiler.h"
#include "Math/Quat.h"
#include "Math/MathFunc.h"

#include <QDomDocument>
#include <cstring>
#include "MemoryLeakCheck.h"

std::string QuatToLegacyRexString(const Quat& q)
{
    char str[256];
    sprintf(str, "%f %f %f %f", q.x, q.y, q.z, q.w);
    return str;
}

Quat QuatFromLegacyRexString(const QString& qStr)
{
    // If consists of 3 components split by spaces, interpret as Euler angles
    if (qStr.split(" ").length() == 3)
    {
        float3 e = DegToRad(float3::FromString(qStr));
        return Quat::FromEulerZYX(e.z, e.y, e.x);
    }
    
    // Else interpret as quaternion directly
    std::string stdStr = qStr.toStdString();
    const char* str = stdStr.c_str();
    if (!str)
        return Quat();
    if (*str == '(')
        ++str;
    Quat q;
    q.w = strtod(str, const_cast<char**>(&str));
    if (*str == ',' || *str == ';')
        ++str;
    q.x = strtod(str, const_cast<char**>(&str));
    if (*str == ',' || *str == ';')
        ++str;
    q.y = strtod(str, const_cast<char**>(&str));
    if (*str == ',' || *str == ';')
        ++str;
    q.z = strtod(str, const_cast<char**>(&str));
    return q;
}

QString modifierMode[] = {
    "relative",
    "absolute",
    "cumulative"
};

AvatarDescAsset::AvatarDescAsset(AssetAPI *owner, const QString &type_, const QString &name_) :
    IAsset(owner, type_, name_)
{
}

AvatarDescAsset::~AvatarDescAsset()
{
    Unload();
}

void AvatarDescAsset::DoUnload()
{
    mesh_ = "";
    skeleton_ = "";
    materials_.clear();
    attachments_.clear();
    boneModifiers_.clear();
    morphModifiers_.clear();
    masterModifiers_.clear();
    animations_.clear();
    properties_.clear();
}

bool AvatarDescAsset::DeserializeFromData(const u8 *data, size_t numBytes, bool /*allowAsynchronous*/)
{
    // Store the raw XML as a string
    QByteArray bytes((const char *)data, numBytes);
    avatarAppearanceXML_ = QString(bytes);
    
    // Then try to parse
    QDomDocument avatarDoc("Avatar");
    // If invalid XML, empty it so we will report IsLoaded == false
    if (!avatarDoc.setContent(avatarAppearanceXML_))
    {
        LogError("Failed to deserialize AvatarDescAsset from data.");
        avatarAppearanceXML_ = "";
    }

    ReadAvatarAppearance(avatarDoc);
    emit AppearanceChanged();

    assetAPI->AssetLoadCompleted(Name());
    return true;
}

bool AvatarDescAsset::SerializeTo(std::vector<u8> &dst, const QString &/*serializationParameters*/) const
{
    QDomDocument avatarDoc("Avatar");
    WriteAvatarAppearance(avatarDoc);
    QByteArray bytes = avatarDoc.toByteArray();
    
    if (!bytes.length())
        return false;
    
    dst.resize(bytes.size());
    memcpy(&dst[0], bytes.data(), bytes.size());
    return true;
}

bool AvatarDescAsset::IsLoaded() const
{
    return !avatarAppearanceXML_.isEmpty();
}

std::vector<AssetReference> AvatarDescAsset::FindReferences() const
{
    std::vector<AssetReference> allRefs;
    AddReference(allRefs, mesh_);
    AddReference(allRefs, skeleton_);
    for (uint i = 0; i < materials_.size(); ++i)
        AddReference(allRefs, materials_[i]);
    for (uint i = 0; i < attachments_.size(); ++i)
    {
        AddReference(allRefs, attachments_[i].mesh_);
        for (uint j = 0; j < attachments_[i].materials_.size(); ++j)
            AddReference(allRefs, attachments_[i].materials_[j]);
    }
    return allRefs;
}

bool AvatarDescAsset::ReadAvatarAppearance(const QDomDocument& source)
{
    PROFILE(Avatar_ReadAvatarAppearance);
    
    QDomElement avatar = source.firstChildElement("avatar");
    if (avatar.isNull())
    {
        LogError("No avatar element");
        return false;
    }

    // Get mesh & skeleton
    QDomElement base_elem = avatar.firstChildElement("base");
    if (!base_elem.isNull())
        mesh_ = base_elem.attribute("mesh");
    
    QDomElement skeleton_elem = avatar.firstChildElement("skeleton");
    if (!skeleton_elem.isNull())
        skeleton_ = skeleton_elem.attribute("name");
    
    // Get materials. Texture override is no longer supported.
    materials_.clear();
    QDomElement material_elem = avatar.firstChildElement("material");
    while (!material_elem.isNull())
    {
        materials_.push_back(material_elem.attribute("name"));
        material_elem = material_elem.nextSiblingElement("material");
    }
    
    // Main transform has never been used, so we don't read it
    
    // Get attachments
    QDomElement attachment_elem = avatar.firstChildElement("attachment");
    attachments_.clear();
    while (!attachment_elem.isNull())
    {
        ReadAttachment(attachment_elem);
        attachment_elem = attachment_elem.nextSiblingElement("attachment");
    }
    
    // Get bone modifiers
    QDomElement bonemodifier_elem = avatar.firstChildElement("dynamic_animation");
    boneModifiers_.clear();
    while (!bonemodifier_elem.isNull())
    {
        ReadBoneModifierSet(bonemodifier_elem);
        bonemodifier_elem = bonemodifier_elem.nextSiblingElement("dynamic_animation");
    }
    // Get bone modifier parameters
    QDomElement bonemodifierparam_elem = avatar.firstChildElement("dynamic_animation_parameter");
    while (!bonemodifierparam_elem.isNull())
    {
        ReadBoneModifierParameter(bonemodifierparam_elem);
        bonemodifierparam_elem = bonemodifierparam_elem.nextSiblingElement("dynamic_animation_parameter");
    }
    
    // Get morph modifiers
    QDomElement morphmodifier_elem = avatar.firstChildElement("morph_modifier");
    morphModifiers_.clear();
    while (!morphmodifier_elem.isNull())
    {
        ReadMorphModifier(morphmodifier_elem);
        morphmodifier_elem = morphmodifier_elem.nextSiblingElement("morph_modifier");
    }
    
    // Get master modifiers
    QDomElement mastermodifier_elem = avatar.firstChildElement("master_modifier");
    masterModifiers_.clear();
    while (!mastermodifier_elem.isNull())
    {
        ReadMasterModifier(mastermodifier_elem);
        mastermodifier_elem = mastermodifier_elem.nextSiblingElement("master_modifier");
    }
    
    // Get animations
    QDomElement animation_elem = avatar.firstChildElement("animation");
    animations_.clear();
    while (!animation_elem.isNull())
    {
        ReadAnimationDefinition(animation_elem);
        animation_elem = animation_elem.nextSiblingElement("animation");
    }
    
    // Get properties
    QDomElement property_elem = avatar.firstChildElement("property");
    properties_.clear();
    while (!property_elem.isNull())
    {
        QString name = property_elem.attribute("name");
        QString value = property_elem.attribute("value");
        if ((name.length()) && (value.length()))
            properties_[name] = value;
        
        property_elem = property_elem.nextSiblingElement("property");
    }
    
    // Refresh slave modifiers
    CalculateMasterModifiers();
    
    // Assetmap not used (deprecated), as asset refs are stored directly
    return true;
}

void AvatarDescAsset::ReadBoneModifierSet(const QDomElement& source)
{
    BoneModifierSet modifier_set;
    modifier_set.name_ = source.attribute("name");
    unsigned num_bones = 0;
    
    QDomElement bones = source.firstChildElement("bones");
    if (!bones.isNull())
    {
        QDomElement bone = bones.firstChildElement("bone");
        while (!bone.isNull())
        {
            BoneModifier modifier;
            modifier.bone_name_ = bone.attribute("name");
            QDomElement rotation = bone.firstChildElement("rotation");
            QDomElement translation = bone.firstChildElement("translation");
            QDomElement scale = bone.firstChildElement("scale");
            
            modifier.start_.position_ = float3::FromString(translation.attribute("start"));
            float3 e = DegToRad(float3::FromString(rotation.attribute("start")));
            modifier.start_.orientation_ = Quat::FromEulerZYX(e.z, e.y, e.x);////ParseEulerAngles(rotation.attribute("start").toStdString());
            modifier.start_.scale_ = float3::FromString(scale.attribute("start"));
            
            modifier.end_.position_ = float3::FromString(translation.attribute("end"));
            e = DegToRad(float3::FromString(rotation.attribute("end").toStdString()));
            modifier.end_.orientation_ = Quat::FromEulerZYX(e.z, e.y, e.x);//ParseEulerAngles(rotation.attribute("end").toStdString());
            modifier.end_.scale_ = float3::FromString(scale.attribute("end"));
            
            std::string trans_mode = translation.attribute("mode").toStdString();
            std::string rot_mode = rotation.attribute("mode").toStdString();
            
            if (trans_mode == "absolute")
                modifier.position_mode_ = BoneModifier::Absolute;
            if (trans_mode == "relative")
                modifier.position_mode_ = BoneModifier::Relative;
            
            if (rot_mode == "absolute")
                modifier.orientation_mode_ = BoneModifier::Absolute;
            if (rot_mode == "relative")
                modifier.orientation_mode_ = BoneModifier::Relative;
            if (rot_mode == "cumulative")
                modifier.orientation_mode_ = BoneModifier::Cumulative;
            
            modifier_set.modifiers_.push_back(modifier);
            
            bone = bone.nextSiblingElement("bone");
            ++num_bones;
        }
    }
    
    if (num_bones)
        boneModifiers_.push_back(modifier_set);
}

void AvatarDescAsset::ReadBoneModifierParameter(const QDomElement& source)
{
    // Find existing modifier from the vector
    QString name = source.attribute("name");
    for (unsigned i = 0; i < boneModifiers_.size(); ++i)
    {
        if (boneModifiers_[i].name_ == name)
        {
            boneModifiers_[i].value_ = source.attribute("position", "0.5").toFloat();
            return;
        }
    }
}

void AvatarDescAsset::ReadMorphModifier(const QDomElement& source)
{
    MorphModifier morph;
    
    morph.name_ = source.attribute("name");
    morph.morph_name_ = source.attribute("internal_name");
    morph.value_ = source.attribute("influence", "0").toFloat();

    morphModifiers_.push_back(morph);
}

void AvatarDescAsset::ReadMasterModifier(const QDomElement& source)
{
    MasterModifier master;
    
    master.name_ = source.attribute("name");
    master.category_ = source.attribute("category");
    master.value_ = source.attribute("position", "0").toFloat();
    
    QDomElement target = source.firstChildElement("target_modifier");
    while (!target.isNull())
    {
        SlaveModifier targetmodifier;
        targetmodifier.name_ = target.attribute("name");
        
        QString targettype = target.attribute("type");
        QString targetmode = target.attribute("mode");
        if (targettype == "morph")
            targetmodifier.type_ = AppearanceModifier::Morph;
        if (targettype == "bone")
            targetmodifier.type_ = AppearanceModifier::Bone;
        if (targettype == "dynamic_animation")
            targetmodifier.type_ = AppearanceModifier::Bone;
            
        QDomElement mapping = target.firstChildElement("position_mapping");
        while (!mapping.isNull())
        {
            SlaveModifier::ValueMapping new_mapping;
            new_mapping.master_ = mapping.attribute("master").toFloat();
            new_mapping.slave_ = mapping.attribute("target").toFloat();
            targetmodifier.mapping_.push_back(new_mapping);
            mapping = mapping.nextSiblingElement("position_mapping");
        }
        
        if (targetmode == "cumulative")
            targetmodifier.mode_ = SlaveModifier::Cumulative;
        else
            targetmodifier.mode_ = SlaveModifier::Average;
        
        master.modifiers_.push_back(targetmodifier);
        
        target = target.nextSiblingElement("target_modifier");
    }
    
    masterModifiers_.push_back(master);
}

void AvatarDescAsset::ReadAnimationDefinitions(const QDomDocument& source)
{
    PROFILE(Avatar_ReadAnimationDefinitions);
    
    animations_.clear();
    
    QDomElement elem = source.firstChildElement("animation");
    while (!elem.isNull())
    {
        ReadAnimationDefinition(elem);
        elem = elem.nextSiblingElement("animation");
    }
}

void AvatarDescAsset::ReadAnimationDefinition(const QDomElement& elem)
{
    if (elem.tagName() != "animation")
        return;

    QString id = elem.attribute("id");
    if (id.isEmpty())
        id = elem.attribute("uuid"); // legacy
    if (id.isEmpty())
    {
        LogError("Missing animation identifier");
        return;
    }
    
    QString intname = elem.attribute("internal_name");
    if (intname.isEmpty())
        intname = elem.attribute("ogrename"); // legacy
    if (intname.isEmpty())
    {
        LogError("Missing mesh animation name");
        return;
    }
    
    AnimationDefinition new_def;
    new_def.id_ = id;
    new_def.animation_name_ = intname;
    new_def.name_ = elem.attribute("name");
    
    new_def.looped_ = ParseBool(elem.attribute("looped", "true"));
    new_def.exclusive_ = ParseBool(elem.attribute("exclusive", "false"));
    new_def.use_velocity_ = ParseBool(elem.attribute("usevelocity", "false"));
    new_def.always_restart_ = ParseBool(elem.attribute("alwaysrestart", "false"));
    new_def.fadein_ = elem.attribute("fadein", "0").toFloat();
    new_def.fadeout_ = elem.attribute("fadeout", "0").toFloat();
    new_def.speedfactor_ = elem.attribute("speedfactor", "1").toFloat();
    new_def.weightfactor_ = elem.attribute("weightfactor", "1").toFloat();
    
    animations_.push_back(new_def);
}

void AvatarDescAsset::ReadAttachment(const QDomElement& elem)
{
    AvatarAttachment attachment;
    
    QDomElement name = elem.firstChildElement("name");
    if (!name.isNull())
        attachment.name_ = name.attribute("value");

    // Awesome new feature: we may define material(s) for attachment
    QDomElement material = elem.firstChildElement("material");
    while (!material.isNull())
    {
        attachment.materials_.push_back(material.attribute("name"));
        material = material.nextSiblingElement("material");
    }

    QDomElement category = elem.firstChildElement("category");
    if (!category.isNull())
    {
        attachment.category_ = category.attribute("name");
    }
    
    QDomElement mesh = elem.firstChildElement("mesh");
    if (!mesh.isNull())
    {
        attachment.mesh_ = mesh.attribute("name");
        attachment.link_skeleton_ = ParseBool(mesh.attribute("linkskeleton"));
    }
    else
    {
        LogError("Attachment without mesh element");
        return;
    }
    
    QDomElement avatar = elem.firstChildElement("avatar");
    if (!avatar.isNull())
    {
        QDomElement bone = avatar.firstChildElement("bone");
        if (!bone.isNull())
        {
            attachment.bone_name_ = bone.attribute("name");
            if (attachment.bone_name_ == "None")
                attachment.bone_name_ = QString();
            if (!bone.attribute("offset").isNull())
                attachment.transform_.position_ = float3::FromString(bone.attribute("offset"));
            if (!bone.attribute("rotation").isNull())
                attachment.transform_.orientation_ = QuatFromLegacyRexString(bone.attribute("rotation"));
            if (!bone.attribute("scale").isNull())
                attachment.transform_.scale_ = float3::FromString(bone.attribute("scale"));
        }
        
        QDomElement polygon = avatar.firstChildElement("avatar_polygon");
        while (!polygon.isNull())
        {
            uint idx = polygon.attribute("idx").toUInt();
            attachment.vertices_to_hide_.push_back(idx);
            polygon = polygon.nextSiblingElement("avatar_polygon");
        }
    }
    else
    {
        LogError("Attachment without avatar element");
        return;
    }
    
    attachments_.push_back(attachment);
}

void AvatarDescAsset::SetMasterModifierValue(const QString& name, float value)
{
    for(uint i = 0; i < masterModifiers_.size(); ++i)
        if (masterModifiers_[i].name_ == name)
        {
            masterModifiers_[i].value_ = Clamp(value, 0.0f, 1.0f);
            for (uint j = 0; j < masterModifiers_[i].modifiers_.size(); ++j)
            {
                AppearanceModifier* mod = FindModifier(masterModifiers_[i].modifiers_[j].name_, masterModifiers_[i].modifiers_[j].type_);
                if (mod)
                    mod->manual_ = false;
            }
            CalculateMasterModifiers();
            emit DynamicAppearanceChanged();
            return;
        }
}

void AvatarDescAsset::SetModifierValue(const QString& name, float value)
{
    value = Clamp(value, 0.0f, 1.0f);

    // Check first for a morph, then for bone
    AppearanceModifier* mod = FindModifier(name, AppearanceModifier::Morph);
    if (mod)
    {
        mod->value_ = value;
        mod->manual_ = true;
        emit DynamicAppearanceChanged();
        return;
    }
    mod = FindModifier(name, AppearanceModifier::Bone);
    if (mod)
    {
        mod->value_ = value;
        mod->manual_ = true;
        emit DynamicAppearanceChanged();
    }
}

void AvatarDescAsset::SetMaterial(uint index, const QString& ref)
{
    if (index >= materials_.size())
        return;
    materials_[index] = ref;
    
    AssetReferencesChanged();
}

void AvatarDescAsset::RemoveAttachment(uint index)
{
    if (index < attachments_.size())
    {
        attachments_.erase(attachments_.begin() + index);
        emit AppearanceChanged();
    }
    else
        LogError("Failed to remove attachment at index " + QString::number(index) + "! Only " + attachments_.size() + "  attachments exist on the avatar asset!");
}

void AvatarDescAsset::RemoveAttachmentsByCategory(QString category)
{
    std::vector<int> toRemove;

    for (uint i = 0; i < attachments_.size(); i++)
    {
        if (attachments_[i].category_ == category)
        {
            toRemove.push_back(i);
        }
    }

    // Remove the attachments, starting from the end of the vector.
    for (int i = toRemove.size()-1; i >= 0; --i)
        RemoveAttachment(toRemove[i]);
}

void AvatarDescAsset::AddAttachment(AssetPtr assetPtr)
{
    std::vector<u8> data;
    bool success = assetPtr->SerializeTo(data);
    if (!success || data.size() == 0)
    {
        LogError("AvatarDescAssett::AddAttachment: Could not serialize attachment");
        return;
    }

    QString string = QString::fromUtf8((char*)&data[0], data.size());

    QDomDocument attachDoc("Attachment");
    if (!attachDoc.setContent(string))
    {
        LogError("AvatarDescAsset::AddAttachment: Could not parse attachment data");
        return;
    }

    QDomElement elem = attachDoc.firstChildElement("attachment");

    if (!elem.isNull())
    {
        ReadAttachment(elem);
        AssetReferencesChanged();
        emit AppearanceChanged();
    }
    else
    {
        LogError("AvatarDescAsset::AddAttachment: Null attachment");
    }
}

bool AvatarDescAsset::HasProperty(const QString &name) const
{
    QMap<QString, QString>::const_iterator i = properties_.find(name);
    if (i == properties_.end())
        return false;
    return i.value().length() > 0;
}

const QString& AvatarDescAsset::GetProperty(const QString& name)
{
    return properties_[name];
}

void AvatarDescAsset::AssetReferencesChanged()
{
    unsigned assets = FindReferences().size();
    // If no references (unlikely), send AppearanceChanged() immediately
    if (!assets)
        emit AppearanceChanged();
    else
    {
        // Otherwise request the (possibly new) assets
        assetAPI->RequestAssetDependencies(this->shared_from_this());
    }
}

void AvatarDescAsset::DependencyLoaded(AssetPtr dependee)
{
    IAsset::DependencyLoaded(dependee);
    
    // Emit AppearanceChanged() when all references have been loaded, and the avatar description is ready to use
    if (!assetAPI->HasPendingDependencies(this->shared_from_this()))
        emit AppearanceChanged();
}

void AvatarDescAsset::CalculateMasterModifiers()
{
    for(uint i = 0; i < morphModifiers_.size(); ++i)
        morphModifiers_[i].ResetAccumulation();

    for(uint i = 0; i < boneModifiers_.size(); ++i)
        boneModifiers_[i].ResetAccumulation();

    for(uint i = 0; i < masterModifiers_.size(); ++i)
        for(uint j = 0; j < masterModifiers_[i].modifiers_.size(); ++j)
        {
            AppearanceModifier* mod = FindModifier(masterModifiers_[i].modifiers_[j].name_, masterModifiers_[i].modifiers_[j].type_);
            if (mod)
            {
                float slave_value = masterModifiers_[i].modifiers_[j].GetMappedValue(masterModifiers_[i].value_);
                mod->AccumulateValue(slave_value, masterModifiers_[i].modifiers_[j].mode_ == SlaveModifier::Average);
            }
        }
}

AppearanceModifier* AvatarDescAsset::FindModifier(const QString & name, AppearanceModifier::ModifierType type)
{
    for(uint i = 0; i < morphModifiers_.size(); ++i)
        if (morphModifiers_[i].name_ == name && morphModifiers_[i].type_ == type)
            return &morphModifiers_[i];
    for (uint i = 0; i < boneModifiers_.size(); ++i)
        if (boneModifiers_[i].name_ == name && boneModifiers_[i].type_ == type)
            return &boneModifiers_[i];
    return 0;
}

void AvatarDescAsset::AddReference(std::vector<AssetReference>& refs, const QString& ref) const
{
    if (ref.length())
    {
        AssetReference newRef(assetAPI->ResolveAssetRef(Name(), ref));
        refs.push_back(newRef);
    }
}

void AvatarDescAsset::WriteAvatarAppearance(QDomDocument& dest) const
{
    // Avatar element
    QDomElement avatar = dest.createElement("avatar");
    
    // Version element
    {
        QDomElement version = dest.createElement("version");
        QDomText text = dest.createTextNode("0.2");
        version.appendChild(text);
        avatar.appendChild(version);
    }
    
    // Mesh element
    {
        QDomElement mesh = dest.createElement("base");
        mesh.setAttribute("name", "default");
        mesh.setAttribute("mesh", mesh_);
        avatar.appendChild(mesh);
    }
    
    // Skeleton element
    if (skeleton_.length())
    {
        QDomElement skeleton = dest.createElement("skeleton");
        skeleton.setAttribute("name", skeleton_);
        avatar.appendChild(skeleton);
    }
    
    // Material elements
    for (uint i = 0; i < materials_.size(); ++i)
    {
        // Append elements in submesh order
        QDomElement material = dest.createElement("material");
        material.setAttribute("name", materials_[i]);
        
        avatar.appendChild(material);
    }
    
    // Attachments
    for (uint i = 0; i < attachments_.size(); ++i)
    {
        QDomElement attachment = WriteAttachment(dest, attachments_[i], mesh_);
        avatar.appendChild(attachment);
    }
    
    // Bone modifiers
    for (uint i = 0; i < boneModifiers_.size(); ++i)
        WriteBoneModifierSet(dest, avatar, boneModifiers_[i]);
    
    // Morph modifiers
    for (uint i = 0; i < morphModifiers_.size(); ++i)
    {
        QDomElement morph = WriteMorphModifier(dest, morphModifiers_[i]);
        avatar.appendChild(morph);
    }
    
    // Master modifiers
    for (uint i = 0; i < masterModifiers_.size(); ++i)
    {
        QDomElement master = WriteMasterModifier(dest, masterModifiers_[i]);
        avatar.appendChild(master);
    }
    
    // Animations
    for (uint i = 0; i < animations_.size(); ++i)
    {
        QDomElement anim = WriteAnimationDefinition(dest, animations_[i]);
        avatar.appendChild(anim);
        ++i;
    }
    
    // Properties
    QMap<QString, QString>::const_iterator i = properties_.begin();
    while (i != properties_.end())
    {
        QDomElement prop = dest.createElement("property");
        prop.setAttribute("name", i.key());
        prop.setAttribute("value", i.value());
        avatar.appendChild(prop);
        ++i;
    }

    dest.appendChild(avatar);
}

QDomElement AvatarDescAsset::WriteAnimationDefinition(QDomDocument& dest, const AnimationDefinition& anim) const
{
    QDomElement elem = dest.createElement("animation");
    
    elem.setAttribute("name", anim.name_);
    elem.setAttribute("id", anim.id_);
    elem.setAttribute("internal_name", anim.animation_name_);
    elem.setAttribute("looped", anim.looped_);
    elem.setAttribute("usevelocity", anim.use_velocity_);
    elem.setAttribute("alwaysrestart", anim.always_restart_);
    elem.setAttribute("fadein", anim.fadein_);
    elem.setAttribute("fadeout", anim.fadeout_);
    elem.setAttribute("speedfactor", anim.speedfactor_);
    elem.setAttribute("weightfactor", anim.weightfactor_);
    
    return elem;
}

void AvatarDescAsset::WriteBoneModifierSet(QDomDocument& dest, QDomElement& dest_elem, const BoneModifierSet& bones) const
{
    QDomElement parameter = dest.createElement("dynamic_animation_parameter");
    QDomElement modifier = dest.createElement("dynamic_animation");
    
    parameter.setAttribute("name", bones.name_);
    parameter.setAttribute("position", bones.value_);
    modifier.setAttribute("name", bones.name_);

    QDomElement base_animations = dest.createElement("base_animations");
    modifier.appendChild(base_animations);
    
    QDomElement bonelist = dest.createElement("bones");
    for (uint i = 0; i < bones.modifiers_.size(); ++i)
    {
        QDomElement bone = WriteBone(dest, bones.modifiers_[i]);
        bonelist.appendChild(bone);
    }
    modifier.appendChild(bonelist);
    
    if (!dest_elem.isNull())
    {
        dest_elem.appendChild(parameter);
        dest_elem.appendChild(modifier);
    }
    else
    {
        dest.appendChild(parameter);
        dest.appendChild(modifier);
    }
}

QDomElement AvatarDescAsset::WriteBone(QDomDocument& dest, const BoneModifier& bone) const
{
    QDomElement elem = dest.createElement("bone");
    elem.setAttribute("name", bone.bone_name_);
    
    QDomElement rotation = dest.createElement("rotation");
    float3 e = RadToDeg(bone.start_.orientation_.ToEulerZYX());
    rotation.setAttribute("start", float3(e.z, e.y, e.x).ToString().c_str()); //WriteEulerAngles(bone.start_.orientation_));
    e = RadToDeg(bone.end_.orientation_.ToEulerZYX());
    rotation.setAttribute("end", float3(e.z, e.y, e.x).ToString().c_str()); //WriteEulerAngles(bone.end_.orientation_));
    rotation.setAttribute("mode", modifierMode[bone.orientation_mode_]);
    
    QDomElement translation = dest.createElement("translation");
    translation.setAttribute("start", bone.start_.position_.SerializeToString().c_str());
    translation.setAttribute("end", bone.end_.position_.SerializeToString().c_str());
    translation.setAttribute("mode", modifierMode[bone.position_mode_]);

    QDomElement scale = dest.createElement("scale");
    scale.setAttribute("start", bone.start_.scale_.SerializeToString().c_str());
    scale.setAttribute("end", bone.end_.scale_.SerializeToString().c_str());

    elem.appendChild(rotation);
    elem.appendChild(translation);
    elem.appendChild(scale);

    return elem;
}

QDomElement AvatarDescAsset::WriteMorphModifier(QDomDocument& dest, const MorphModifier& morph) const
{
    QDomElement elem = dest.createElement("morph_modifier");
    elem.setAttribute("name", morph.name_);
    elem.setAttribute("internal_name", morph.morph_name_);
    elem.setAttribute("influence", morph.value_);
    
    return elem;
}

QDomElement AvatarDescAsset::WriteMasterModifier(QDomDocument& dest, const MasterModifier& master) const
{
    QDomElement elem = dest.createElement("master_modifier");
    elem.setAttribute("name", master.name_);
    elem.setAttribute("position", master.value_);
    elem.setAttribute("category", master.category_);
    for (uint i = 0; i < master.modifiers_.size(); ++i)
    {
        QDomElement target_elem = dest.createElement("target_modifier");
        target_elem.setAttribute("name", master.modifiers_[i].name_);
        if (master.modifiers_[i].type_ == AppearanceModifier::Morph)
            target_elem.setAttribute("type", "morph");
        else
            target_elem.setAttribute("type", "dynamic_animation");
        if (master.modifiers_[i].mode_ == SlaveModifier::Cumulative)
            target_elem.setAttribute("mode", "cumulative");
        else
            target_elem.setAttribute("mode", "average");
        for(uint j = 0; j < master.modifiers_[i].mapping_.size(); ++j)
        {
            QDomElement mapping_elem = dest.createElement("position_mapping");
            mapping_elem.setAttribute("master", master.modifiers_[i].mapping_[j].master_);
            mapping_elem.setAttribute("target", master.modifiers_[i].mapping_[j].slave_);
            target_elem.appendChild(mapping_elem);
        }
        elem.appendChild(target_elem);
    }
    
    return elem;
}

QDomElement AvatarDescAsset::WriteAttachment(QDomDocument& dest, const AvatarAttachment& attachment, const QString& mesh) const
{
    QDomElement elem = dest.createElement("attachment");
    
    QDomElement name_elem = dest.createElement("name");
    name_elem.setAttribute("value", attachment.name_);
    elem.appendChild(name_elem);
    
    QDomElement mesh_elem = dest.createElement("mesh");
    mesh_elem.setAttribute("name", attachment.mesh_);
    int link = 0;
    if (attachment.link_skeleton_)
        link = 1;
    mesh_elem.setAttribute("linkskeleton", link);
    elem.appendChild(mesh_elem);
    
    for(unsigned i = 0; i < attachment.materials_.size(); ++i)
    {
        QDomElement material_elem = dest.createElement("material");
        material_elem.setAttribute("name", attachment.materials_[i]);
        elem.appendChild(material_elem);
    }
    
    QDomElement category_elem = dest.createElement("category");
    category_elem.setAttribute("name", attachment.category_);
    elem.appendChild(category_elem);
    
    QDomElement avatar_elem = dest.createElement("avatar");
    avatar_elem.setAttribute("name", mesh);
    
    {
        QString boneName = attachment.bone_name_;
        if (boneName.isEmpty())
            boneName= "None";

        QDomElement bone_elem = dest.createElement("bone");
        bone_elem.setAttribute("name", boneName);
        bone_elem.setAttribute("offset", attachment.transform_.position_.SerializeToString().c_str());
        bone_elem.setAttribute("rotation", attachment.transform_.orientation_.SerializeToStringWXYZ().c_str());
        bone_elem.setAttribute("scale", attachment.transform_.scale_.SerializeToString().c_str());

        avatar_elem.appendChild(bone_elem);

        for(uint i = 0; i < attachment.vertices_to_hide_.size(); ++i)
        {
            QDomElement polygon_elem = dest.createElement("avatar_polygon");
            polygon_elem.setAttribute("idx", (int)attachment.vertices_to_hide_[i]);
            avatar_elem.appendChild(polygon_elem);
        }
    }
    elem.appendChild(avatar_elem);
    
    return elem;
}

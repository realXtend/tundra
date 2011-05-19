// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "AvatarDescAsset.h"
#include "AvatarModule.h"
#include "AssetAPI.h"
#include "XMLUtilities.h"
#include "Profiler.h"

#include <QDomDocument>
#include <cstring>
#include "MemoryLeakCheck.h"

using namespace Avatar;

std::string modifierMode[] = {
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

bool AvatarDescAsset::DeserializeFromData(const u8 *data, size_t numBytes)
{
    // Store the raw XML as a string
    QByteArray bytes((const char *)data, numBytes);
    avatarAppearanceXML_ = QString(bytes);
    
    // Then try to parse
    QDomDocument avatarDoc("Avatar");
    // If invalid XML, empty it so we will report IsLoaded == false
    if (!avatarDoc.setContent(avatarAppearanceXML_))
        avatarAppearanceXML_ = "";
    ReadAvatarAppearance(avatarDoc);
    
    emit AppearanceChanged();
    return true;
}

bool AvatarDescAsset::SerializeTo(std::vector<u8> &dst, const QString &serializationParameters) const
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
    modifier_set.name_ = source.attribute("name").toStdString();
    unsigned num_bones = 0;
    
    QDomElement bones = source.firstChildElement("bones");
    if (!bones.isNull())
    {
        QDomElement bone = bones.firstChildElement("bone");
        while (!bone.isNull())
        {
            BoneModifier modifier;
            modifier.bone_name_ = bone.attribute("name").toStdString();
            QDomElement rotation = bone.firstChildElement("rotation");
            QDomElement translation = bone.firstChildElement("translation");
            QDomElement scale = bone.firstChildElement("scale");
            
            modifier.start_.position_ = ParseVector3(translation.attribute("start").toStdString());
            modifier.start_.orientation_ = ParseEulerAngles(rotation.attribute("start").toStdString());
            modifier.start_.scale_ = ParseVector3(scale.attribute("start").toStdString());
            
            modifier.end_.position_ = ParseVector3(translation.attribute("end").toStdString());
            modifier.end_.orientation_ = ParseEulerAngles(rotation.attribute("end").toStdString());
            modifier.end_.scale_ = ParseVector3(scale.attribute("end").toStdString());
            
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
    std::string name = source.attribute("name").toStdString();
    for (unsigned i = 0; i < boneModifiers_.size(); ++i)
    {
        if (boneModifiers_[i].name_ == name)
        {
            boneModifiers_[i].value_ = ParseReal(source.attribute("position", "0.5").toStdString());
            return;
        }
    }
}

void AvatarDescAsset::ReadMorphModifier(const QDomElement& source)
{
    MorphModifier morph;
    
    morph.name_ = source.attribute("name").toStdString();
    morph.morph_name_ = source.attribute("internal_name").toStdString();
    morph.value_ = ParseReal(source.attribute("influence", "0").toStdString());

    morphModifiers_.push_back(morph);
}

void AvatarDescAsset::ReadMasterModifier(const QDomElement& source)
{
    MasterModifier master;
    
    master.name_ = source.attribute("name").toStdString();
    master.category_ = source.attribute("category").toStdString();
    master.value_ = ParseReal(source.attribute("position", "0").toStdString());
    
    QDomElement target = source.firstChildElement("target_modifier");
    while (!target.isNull())
    {
        SlaveModifier targetmodifier;
        targetmodifier.name_ = target.attribute("name").toStdString();
        
        std::string targettype = target.attribute("type").toStdString();
        std::string targetmode = target.attribute("mode").toStdString();
                 
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
            new_mapping.master_ = ParseReal(mapping.attribute("master").toStdString());
            new_mapping.slave_ = ParseReal(mapping.attribute("target").toStdString());
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
    
    std::string id = elem.attribute("id").toStdString();
    if (id.empty())
        id = elem.attribute("uuid").toStdString(); // legacy
    if (id.empty())
    {
        LogError("Missing animation identifier");
        return;
    }
    
    std::string intname = elem.attribute("internal_name").toStdString();
    if (intname.empty())
        intname = elem.attribute("ogrename").toStdString(); // legacy
    if (intname.empty())
    {
        LogError("Missing mesh animation name");
        return;
    }
    
    AnimationDefinition new_def;
    new_def.id_ = id;
    new_def.animation_name_ = intname;
    new_def.name_ = elem.attribute("name").toStdString();
    
    new_def.looped_ = ParseBool(elem.attribute("looped", "true").toStdString());
    new_def.exclusive_ = ParseBool(elem.attribute("exclusive", "false").toStdString());
    new_def.use_velocity_ = ParseBool(elem.attribute("usevelocity", "false").toStdString());
    new_def.always_restart_ = ParseBool(elem.attribute("alwaysrestart", "false").toStdString());
    new_def.fadein_ = ParseReal(elem.attribute("fadein", "0").toStdString());
    new_def.fadeout_ = ParseReal(elem.attribute("fadeout", "0").toStdString());
    new_def.speedfactor_ = ParseReal(elem.attribute("speedfactor", "1").toStdString());
    new_def.weightfactor_ = ParseReal(elem.attribute("weightfactor", "1").toStdString());
    
    animations_.push_back(new_def);
}

void AvatarDescAsset::ReadAttachment(const QDomElement& elem)
{
    AvatarAttachment attachment;
    
    QDomElement name = elem.firstChildElement("name");
    if (!name.isNull())
    {
        attachment.name_ = name.attribute("value").toStdString();
    }
    else
    {
        LogError("Attachment without name element");
        return;
    }
    
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
        attachment.category_ = category.attribute("name").toStdString();
    }
    
    QDomElement mesh = elem.firstChildElement("mesh");
    if (!mesh.isNull())
    {
        attachment.mesh_ = mesh.attribute("name");
        attachment.link_skeleton_ = ParseBool(mesh.attribute("linkskeleton").toStdString());
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
            attachment.bone_name_ = bone.attribute("name").toStdString();
            if (attachment.bone_name_ == "None")
                attachment.bone_name_ = std::string();
            attachment.transform_.position_ = ParseVector3(bone.attribute("offset").toStdString());
            attachment.transform_.orientation_ = ParseQuaternion(bone.attribute("rotation").toStdString());
            attachment.transform_.scale_ = ParseVector3(bone.attribute("scale").toStdString());
        }
        
        QDomElement polygon = avatar.firstChildElement("avatar_polygon");
        while (!polygon.isNull())
        {
            uint idx = ParseInt(polygon.attribute("idx").toStdString());
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
    std::string nameStd = name.toStdString();
    
    if (value < 0.0) value = 0.0;
    if (value > 1.0) value = 1.0;
    
    for (uint i = 0; i < masterModifiers_.size(); ++i)
    {
        if (masterModifiers_[i].name_ == nameStd)
        {
            masterModifiers_[i].value_ = value;
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
}

void AvatarDescAsset::SetModifierValue(const QString& name, float value)
{
    std::string nameStd = name.toStdString();
    
    if (value < 0.0) value = 0.0;
    if (value > 1.0) value = 1.0;
    
    // Check first for a morph, then for bone
    AppearanceModifier* mod = FindModifier(nameStd, AppearanceModifier::Morph);
    if (mod)
    {
        mod->value_ = value;
        mod->manual_ = true;
        emit DynamicAppearanceChanged();
        return;
    }
    mod = FindModifier(nameStd, AppearanceModifier::Bone);
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

bool AvatarDescAsset::HasProperty(QString name) const
{
    QMap<QString, QString>::const_iterator i = properties_.find(name);
    if (i == properties_.end())
        return false;
    const QString& value = i.value();
    return value.length() > 0;
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
    if (assetAPI->NumPendingDependencies(this->shared_from_this()) == 0)
        emit AppearanceChanged();
}

void AvatarDescAsset::CalculateMasterModifiers()
{
    for (uint i = 0; i < morphModifiers_.size(); ++i)
        morphModifiers_[i].ResetAccumulation();

    for (uint i = 0; i < boneModifiers_.size(); ++i)
        boneModifiers_[i].ResetAccumulation();

    for (uint i = 0; i < masterModifiers_.size(); ++i)
    {
        for (uint j = 0; j < masterModifiers_[i].modifiers_.size(); ++j)
        {
            AppearanceModifier* mod = FindModifier(masterModifiers_[i].modifiers_[j].name_, masterModifiers_[i].modifiers_[j].type_);
            if (mod)
            {
                float slave_value = masterModifiers_[i].modifiers_[j].GetMappedValue(masterModifiers_[i].value_);
                
                mod->AccumulateValue(slave_value, masterModifiers_[i].modifiers_[j].mode_ == SlaveModifier::Average);
            }
        }
    }
}

AppearanceModifier* AvatarDescAsset::FindModifier(const std::string& name, AppearanceModifier::ModifierType type)
{
    for (uint i = 0; i < morphModifiers_.size(); ++i)
    {
        if ((morphModifiers_[i].name_ == name) && (morphModifiers_[i].type_ == type))
            return &morphModifiers_[i];
    }
    for (uint i = 0; i < boneModifiers_.size(); ++i)
    {
        if ((boneModifiers_[i].name_ == name) && (boneModifiers_[i].type_ == type))
            return &boneModifiers_[i];
    }
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
    
    SetAttribute(elem, "name", anim.name_);
    SetAttribute(elem, "id", anim.id_);
    SetAttribute(elem, "internal_name", anim.animation_name_);
    SetAttribute(elem, "looped", anim.looped_);
    SetAttribute(elem, "usevelocity", anim.use_velocity_);
    SetAttribute(elem, "alwaysrestart", anim.always_restart_);
    SetAttribute(elem, "fadein", anim.fadein_);
    SetAttribute(elem, "fadeout", anim.fadeout_);
    SetAttribute(elem, "speedfactor", anim.speedfactor_);
    SetAttribute(elem, "weightfactor", anim.weightfactor_);
    
    return elem;
}

void AvatarDescAsset::WriteBoneModifierSet(QDomDocument& dest, QDomElement& dest_elem, const BoneModifierSet& bones) const
{
    QDomElement parameter = dest.createElement("dynamic_animation_parameter");
    QDomElement modifier = dest.createElement("dynamic_animation");
    
    SetAttribute(parameter, "name", bones.name_);
    SetAttribute(parameter, "position", bones.value_);
    SetAttribute(modifier, "name", bones.name_);

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
    SetAttribute(elem, "name", bone.bone_name_);
    
    QDomElement rotation = dest.createElement("rotation");
    SetAttribute(rotation, "start", WriteEulerAngles(bone.start_.orientation_));
    SetAttribute(rotation, "end", WriteEulerAngles(bone.end_.orientation_));
    SetAttribute(rotation, "mode", modifierMode[bone.orientation_mode_]);
    
    QDomElement translation = dest.createElement("translation");
    SetAttribute(translation, "start", WriteVector3(bone.start_.position_));
    SetAttribute(translation, "end", WriteVector3(bone.end_.position_));
    SetAttribute(translation, "mode", modifierMode[bone.position_mode_]);

    QDomElement scale = dest.createElement("scale");
    SetAttribute(scale, "start", WriteVector3(bone.start_.scale_));
    SetAttribute(scale, "end", WriteVector3(bone.end_.scale_));
    
    elem.appendChild(rotation);
    elem.appendChild(translation);
    elem.appendChild(scale);

    return elem;
}

QDomElement AvatarDescAsset::WriteMorphModifier(QDomDocument& dest, const MorphModifier& morph) const
{
    QDomElement elem = dest.createElement("morph_modifier");
    SetAttribute(elem, "name", morph.name_);
    SetAttribute(elem, "internal_name", morph.morph_name_);
    SetAttribute(elem, "influence", morph.value_);
    
    return elem;
}

QDomElement AvatarDescAsset::WriteMasterModifier(QDomDocument& dest, const MasterModifier& master) const
{
    QDomElement elem = dest.createElement("master_modifier");
    SetAttribute(elem, "name", master.name_);
    SetAttribute(elem, "position", master.value_);
    SetAttribute(elem, "category", master.category_);
    for (uint i = 0; i < master.modifiers_.size(); ++i)
    {
        QDomElement target_elem = dest.createElement("target_modifier");
        SetAttribute(target_elem, "name", master.modifiers_[i].name_);
        if (master.modifiers_[i].type_ == AppearanceModifier::Morph)
            SetAttribute(target_elem, "type", "morph");
        else
            SetAttribute(target_elem, "type", "dynamic_animation");
        if (master.modifiers_[i].mode_ == SlaveModifier::Cumulative)
            SetAttribute(target_elem, "mode", "cumulative");
        else
            SetAttribute(target_elem, "mode", "average");
        for (uint j = 0; j < master.modifiers_[i].mapping_.size(); ++j)
        {
            QDomElement mapping_elem = dest.createElement("position_mapping");
            SetAttribute(mapping_elem, "master", master.modifiers_[i].mapping_[j].master_);
            SetAttribute(mapping_elem, "target", master.modifiers_[i].mapping_[j].slave_);
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
    SetAttribute(name_elem, "value", attachment.name_);
    elem.appendChild(name_elem);
    
    QDomElement mesh_elem = dest.createElement("mesh");
    mesh_elem.setAttribute("name", mesh);
    int link = 0;
    if (attachment.link_skeleton_) 
        link = 1;
    SetAttribute(mesh_elem, "linkskeleton", link);
    elem.appendChild(mesh_elem);
    
    for (unsigned i = 0; i < attachment.materials_.size(); ++i)
    {
        QDomElement material_elem = dest.createElement("material");
        material_elem.setAttribute("name", attachment.materials_[i]);
        elem.appendChild(material_elem);
    }
    
    QDomElement category_elem = dest.createElement("category");
    SetAttribute(category_elem, "name", attachment.category_);
    elem.appendChild(category_elem);
    
    QDomElement avatar_elem = dest.createElement("avatar");
    avatar_elem.setAttribute("name", mesh);
    
    {
        std::string bonename = attachment.bone_name_;
        if (bonename.empty())
            bonename = "None";
        
        QDomElement bone_elem = dest.createElement("bone");
        SetAttribute(bone_elem, "name", bonename);
        SetAttribute(bone_elem, "offset", WriteVector3(attachment.transform_.position_));
        SetAttribute(bone_elem, "rotation", WriteQuaternion(attachment.transform_.orientation_));
        SetAttribute(bone_elem, "scale", WriteVector3(attachment.transform_.scale_));
        
        avatar_elem.appendChild(bone_elem);
        
        for (uint i = 0; i < attachment.vertices_to_hide_.size(); ++i)
        {
            QDomElement polygon_elem = dest.createElement("avatar_polygon");
            SetAttribute(polygon_elem, "idx", (int)attachment.vertices_to_hide_[i]);
            avatar_elem.appendChild(polygon_elem);
        }
    }
    elem.appendChild(avatar_elem);
    
    return elem;
}

// For conditions of distribution and use, see copyright notice in license.txt

//! Rex avatar base class. Contains all non-renderer, non-protocol specific aspects of the rex avatar
class Avatar
{
public:
    //! Minimum avatar version that this class provides functionality for
    static const float AVATAR_BASE_VERSION;

    //! default constructor. A generic avatar.
    Avatar();
      
    //! Constructor that takes unique identifier for the avatar
    Avatar(const std::string &uid);

    //! Resets the avatar to basic state (no materials, attachment...). Keeps the id, if any, as that is still probably needed.
    virtual void clear(bool deleteAttachments = true);

    //! removes all attachments
    virtual void clearAttachments();

    //! returns name of the 3D model used by the avatar
    virtual std::string Avatar::getMesh() const;

    //! Set name of the 3D model to use for this avatar
    virtual void setMesh(const std::string &mesh);

    //! Setter and getter for skeleton name of the avatar
    virtual const std::string &getSkeleton() const;
    virtual void setSkeleton(const std::string &skeleton);

    //! Attach a new mesh to this avatar
    virtual AvatarAttachment *addAttachment(AvatarAttachment *attachment);

    //! Set version the avatar should be based on. (Imported avatar data is from this version)
    void Avatar::setVersion(float version);

    //! Returns current avatar version. AVATAR_BASE_VERSION by default.
    float Avatar::getVersion() const;

    //! Adds new material with name 'name'
    /*! This is more generic way of adding materials to avatar and doesn't
          depend on specific texture placement. These materials will not be serialized,
          unless directly applied to the avatar entity.

          \return The index of the added material
    */
    virtual size_t addMaterial(const std::string &name);

    //! Sets material to specific index position
    /*!
          This is the more specific way of adding material to avatar. If these material assignments
          exist, they are used in serializing instead of using the actual materials the avatar might
          be using.
    */
    virtual void setMaterial(size_t index, const std::string &name);

    //! Returns material in specific index position. It is important to
    //! make sure index is smaller than number of materials.
    const std::string &getMaterial(size_t index) const;

    //! Returns number of materials
    size_t getNumMaterials() const;

    //! Returns number of attachments associated with this avatar. They may not actually be attached.
    virtual size_t getNumAttachments() const;

    //! Returns an attachment associated with this avatar. The attachment may not actually be attached.
    virtual const AvatarAttachment *getAttachment(size_t index) const;

    //! Sets texture for avatar
    /*!
      \param position placement of the texture, such as FACE or BODY
      \param name name of the texture
    */
    virtual void setTexture(TEXTURE position, const std::string &name);

    //! Returns texture at position
    virtual std::string getTexture(TEXTURE position);

    //! Sets new animation modifier effect (bonedeform)
    /*!
      \param name (human-readable) name of the dynamic animation
      \param animation Animation modifier to add
    */
    virtual void setAnimationModifier(const std::string &name, const DynamicAnimationModifier &animation);

    //! Returns animation modifier (bonedeform).
    /*! Returns empty animation modifier if one with specified name is not found.
    */
    virtual const DynamicAnimationModifier &getAnimationModifier(const std::string &name);

    //! Returns map (unordered) of dynamic animation modifiers for this avatar
    virtual const AnimationModifierMap &getAnimationModifierMap() const;

    //! Returns vector (ordered) of dynamic animation modifiers for this avatar
    /*! It is important to apply animation modifiers in order, as the end result depends on the order.
    */
    virtual const AnimationModifierVector &getAnimationModifierVector() const;

    //! Set position for animation modifier
    virtual void setAnimationModifierPosition(const std::string &name, float position = 0.5f);

    //! Sets new morph modifier effect
    /*!
      \param name (human-readable) name of the morph animation
      \param morph Morph modifier to add
    */
    virtual void setMorphModifier(const std::string &name, const MorphModifier& morph);

    //! Returns morph modifier.
    /*! Returns empty modifier if one with specified name is not found.
    */
    virtual const MorphModifier& getMorphModifier(const std::string &name);

    //! Returns map of morph modifiers 
    virtual const MorphModifierMap& getMorphModifierMap() const;

    //! Set morph modifier influence
    virtual void setMorphModifierInfluence(const std::string &name, float influence = 0.0f);

    //! Set morph modifier manual status
    virtual void setMorphModifierManual(const std::string &name, bool manual);

    //! Sets new texture modifier
    /*!
      \param name (human-readable) name of the texture modifier
      \param morph Texture modifier to add
    */
    virtual void setTextureModifier(const std::string &name, const TextureModifier& textureMod);

    //! Returns texture modifier.
    /*! Returns empty modifier if one with specified name is not found.
    */
    virtual const TextureModifier& getTextureModifier(const std::string &name);

    //! Returns map of texture modifiers 
    virtual const TextureModifierMap& getTextureModifierMap() const;

    //! Set texture modifier influence
    virtual void setTextureModifierInfluence(const std::string &name, float influence = 0.0f);

    //! Returns animation definition by name
    /*! Returns empty definition if one with specified name is not found.
    */
    virtual const AnimationDef& getAnimationDef(const std::string &name);

    //! Returns map of animation definitions
    virtual const AnimationDefMap& getAnimationDefMap() const;

    //! Returns modifiable map of animation definitions
    virtual AnimationDefMap& getAnimationDefMap();

    //! Set new animation definition
    /*! If one exists with same name, will replace it
    */
    virtual void setAnimationDef(const std::string &name, const AnimationDef& animationDef);

    //! Clear all animation definitions
    virtual void clearAnimationDefs();

    //! Sets new master appearance modifier
    /*!
      \param name (human-readable) name of the master appearance modifier
      \param modifier master appearance modifier to add
    */
    virtual void setMasterModifier(const std::string &name, const MasterModifier& modifier);

    //! Returns master appearance modifier.
    /*! Returns empty modifier if one with specified name is not found.
    */
    virtual const MasterModifier& getMasterModifier(const std::string &name);

    //! Returns map of master appearance modifiers 
    virtual const MasterModifierMap& getMasterModifierMap() const;

    //! Set master modifier position
    virtual void setMasterModifierPosition(const std::string &name, float position = 0.0f);

    //! Clear all master modifiers
    virtual void clearMasterModifiers();

    //! Re-calculate all master modifiers
    virtual void calculateMasterModifiers();

    //! Returns map of bone constraints
    virtual const BoneConstraintMap& getBoneConstraintMap() const;

    //! Clears all bone constraints
    virtual void clearBoneConstraints();

    //! Sets constraints of a certain bone
    virtual void setBoneConstraints(const std::string &bone, const BoneConstraintVector &constraints);

    //! Add a single constraint to a certain bone
    virtual void addBoneConstraint(const std::string &bone, const BoneConstraint &constraint);

    //! Returns number of skeletons that contains extra animation data for the avatar
    virtual size_t getNumExtraAnimatedSkeletons() const;

    //! Returns name of a skeleton that contains extra animation data for the avatar
    virtual const std::string &getExtraAnimatedSkeleton(size_t index) const;

    //! Add skeleton that contains extra animation data for the avatar
    virtual void addExtraAnimatedSkeleton(const std::string &name);

    //! Returns read-only map of properties
    virtual const PropertyMap& getPropertyMap() const;

    //! Sets a generic property
    virtual void setProperty(const std::string& key, const std::string& value);

    //! Whether avatar has non-empty property by certain key
    virtual bool hasProperty(const std::string& key) const;

    //! Gets a property by key. Returns empty string if not found
    virtual const std::string& getProperty(const std::string& key);

    //! Unsets (clears) a generic property
    virtual void unsetProperty(const std::string& key);

    //! Clears all generic properties
    virtual void clearProperties();
};


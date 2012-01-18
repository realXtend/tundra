// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "CoreTypes.h"
#include "OgreModuleApi.h"

#include <OgreMaterial.h>
#include <OgreCompositorInstance.h>

#include <QString>

/// @note This class and its implementation is taken from the Ogre samples
/** @cond PRIVATE */
class HDRListener: public Ogre::CompositorInstance::Listener
{
public:
    HDRListener();
    virtual ~HDRListener();
    void notifyViewportSize(int width, int height);
    void notifyCompositor(Ogre::CompositorInstance* instance);
    virtual void notifyMaterialSetup(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat);
    ///\todo Is this needed? Does nothing atm.
    virtual void notifyMaterialRender(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat) {}

protected:
    int mVpWidth, mVpHeight;
    int mBloomSize;
    // Array params - have to pack in groups of 4 since this is how Cg generates them
    // also prevents dependent texture read problems if ops don't require swizzle
    float mBloomTexWeights[15][4];
    float mBloomTexOffsetsHorz[15][4];
    float mBloomTexOffsetsVert[15][4];
};

///@note This class and its implementation is taken from the Ogre samples
class GaussianListener: public Ogre::CompositorInstance::Listener
{
public:
    GaussianListener();
    virtual ~GaussianListener();
    void notifyViewportSize(int width, int height);
    virtual void notifyMaterialSetup(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat);
    ///\todo Is this needed? Does nothing atm.
    virtual void notifyMaterialRender(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat) {}

protected:
    int mVpWidth, mVpHeight;
    // Array params - have to pack in groups of 4 since this is how Cg generates them
    // also prevents dependent texture read problems if ops don't require swizzle
    float mBloomTexWeights[15][4];
    float mBloomTexOffsetsHorz[15][4];
    float mBloomTexOffsetsVert[15][4];
};
/** @endcond PRIVATE */

/// Handles the post-processing effects
class OGRE_MODULE_API OgreCompositionHandler
{
public:
    OgreCompositionHandler();
    virtual ~OgreCompositionHandler();

    /// Sets viewport.
    /** This has to be called before trying to enable/disable effects */
    void SetViewport(Ogre::Viewport *vp);

    /// Adds specified compositor for the viewport if it's found. Effect is appended last in the chain if position is not specified (Except HDR)
    bool AddCompositorForViewport(const std::string &compositor, Ogre::Viewport *vp, int position = -1);

    /// Remove specified compositor effect from viewport
    void RemoveCompositorFromViewport(const std::string &compositor, Ogre::Viewport *vp);

    /// Convenience function that will add specified compositor for the default viewport given in initialization. HDR will always be first.
    bool AddCompositorForViewport(const std::string &compositor, int position = -1);

    /// Convenience function that will add specified compositor for the default viewport given in initialization, uses priority rather than position
    bool AddCompositorForViewportPriority(const std::string &compositor, int priority = 0);

    /// Convenience funtion to remove specified  compositor from the default viewport
    void RemoveCompositorFromViewport(const std::string &compositor);

    /// Apply a shader parameter to the specified compositor.
    /** The compositor should be enabled of course */
    void SetCompositorParameter(const std::string &compositorName, const QList< std::pair<std::string, Ogre::Vector4> > &source) const;

    /// Returns list of compositor parameter names and their current values in format "name=value".
    /** @note Currently only returns parameters from compositor's composition techniques' output target pass. */
    QStringList CompositorParameters(const std::string &compositorName) const;

    /// Enable or disable a compositor that has already been added to the default viewport
    void SetCompositorEnabled(const std::string &compositor, bool enable) const;

    /// Disable all compositors from the viewport
    void RemoveAllCompositors();

    /// Camera has been changed. Update it to the compositor chain
    void CameraChanged(Ogre::Viewport* vp, Ogre::Camera* newCamera);

    /// Returns list of names for available compositors.
    QStringList AvailableCompositors() const;

private:
    struct Compositor
    {
        std::string name;
        int position;
        bool operator <(const Compositor &rhs) const { return position < rhs.position; }
    };

    /// Adds and enables compositor on viewport
    bool AddCompositor(const std::string &compositor, Ogre::Viewport *vp, int position);

    /// Set gpu program parameters for the specified composition target
    void SetCompositorTargetParameters(Ogre::CompositionTargetPass *target, const QList< std::pair<std::string, Ogre::Vector4> > &source) const;

    /// Set gpu program parameters for the specified material
    void SetMaterialParameters(const Ogre::MaterialPtr &material, const QList< std::pair<std::string, Ogre::Vector4> > &source) const;

    /// Ogre viewport.
    Ogre::Viewport* viewport_;

    /// Framelistener for HDR effect
    HDRListener hdr_listener_;

    /// Framelistener for gaussian blur
    GaussianListener gaussian_listener_;

    /// Stores priorities for compositors. Compositor name is used for the key to make sure each compositor only has one priority.
    std::map<std::string, int> priorities_;
};

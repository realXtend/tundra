/// @file Environment.cpp
/// @brief Manages environment-related reX-logic, e.g. world time and lighting.
/// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EnvironmentModule.h"
#include "Environment.h"
#include "EC_OgreEnvironment.h"
#include "SceneManager.h"
#include "NetworkEvents.h"
#include "NetworkMessages/NetInMessage.h"

#include <QVector>

namespace Environment
{

Environment::Environment(EnvironmentModule *owner) : 
    owner_(owner), 
    activeEnvEntity_(Scene::EntityWeakPtr()), 
    time_override_(false),
    usecSinceStart_(0),
    secPerDay_(0),
    secPerYear_(0),
    sunDirection_(RexTypes::Vector3()),
    sunPhase_(0.0),
    sunAngVelocity_(RexTypes::Vector3())
{}

Environment::~Environment()
{
    // Does not own.
    activeEnvEntity_.reset();
    owner_ = 0;
}

OgreRenderer::EC_OgreEnvironment* Environment::GetEnvironmentComponent()
{
    if (activeEnvEntity_.expired())
        return 0;
    
    OgreRenderer::EC_OgreEnvironment* ec = activeEnvEntity_.lock()->GetComponent<OgreRenderer::EC_OgreEnvironment>().get();  
    return ec;
}


Scene::EntityWeakPtr Environment::GetEnvironmentEntity()
{
    return activeEnvEntity_;
}

void Environment::CreateEnvironment()
{
    if ( !activeEnvEntity_.expired())
        return;

    Scene::ScenePtr active_scene = owner_->GetFramework()->GetDefaultWorldScene();
    Scene::EntityPtr entity = active_scene->CreateEntity(active_scene->GetNextFreeId());
    entity->AddComponent(owner_->GetFramework()->GetComponentManager()->CreateComponent("EC_OgreEnvironment"));
    activeEnvEntity_ = entity;
}

bool Environment::HandleSimulatorViewerTimeMessage(ProtocolUtilities::NetworkEventInboundData *data)
{
    ProtocolUtilities::NetInMessage &msg = *data->message;
    msg.ResetReading();

    ///\ secPerDay,secPerYear, sunPhase seems to be zero, at least with 0.4 server
    try
    {
	    usecSinceStart_ = (time_t)msg.ReadU64();
	    secPerDay_ = msg.ReadU32();
	    secPerYear_ = msg.ReadU32();
	    sunDirection_ = msg.ReadVector3();
	    sunPhase_ = msg.ReadF32();
	    sunAngVelocity_ = msg.ReadVector3();
    }
    catch(NetMessageException &)
    {
		//! todo crashes with 0.4 server add error message.
    }
    
    // Calculate time of day from sun phase, which seems the most reliable way to do it
    float dayphase;
    
    if (sunPhase_ < 1.25 * PI)
        dayphase = (sunPhase_ / PI + 1.0) * 0.33333333;
    else
        dayphase = -0.5 + (sunPhase_ / PI);

    OgreRenderer::EC_OgreEnvironment* env = GetEnvironmentComponent();
    if (!env)
        return false; 

    // Update the sunlight direction and angle velocity.
    ///\note Not needed anymore as we use Caelum now.
    //OgreRenderer::EC_OgreEnvironment &env = *checked_static_cast<OgreRenderer::EC_OgreEnvironment*>
    //    (component.get());
    //env.SetSunDirection(-sunDirection_);

    if (!time_override_)
    {
        env->SetTime(dayphase);
    }

    return false;
}

void Environment::SetWaterFog(float fogStart, float fogEnd, const QVector<float>& color)
{
    OgreRenderer::EC_OgreEnvironment* env = GetEnvironmentComponent();
    if (!env)
        return; 
        
    env->SetWaterFogStart(fogStart);
    env->SetWaterFogEnd(fogEnd);
    Ogre::ColourValue fogColour(color[0], color[1], color[2]);
    env->SetWaterFogColor(fogColour); 
    emit WaterFogAdjusted(fogStart, fogEnd, color);   
}

void Environment::SetGroundFog(float fogStart, float fogEnd, const QVector<float>& color)
{
    OgreRenderer::EC_OgreEnvironment* env = GetEnvironmentComponent();
    if (!env)
        return; 
           
    env->SetGroundFogStart(fogStart);
    env->SetGroundFogEnd(fogEnd);
    Ogre::ColourValue fogColour(color[0], color[1], color[2]);
    env->SetGroundFogColor(fogColour); 
    emit GroundFogAdjusted(fogStart, fogEnd, color);
}

void Environment::SetFogColorOverride(bool enabled)
{
    OgreRenderer::EC_OgreEnvironment* env = GetEnvironmentComponent();
    if (!env)
        return; 
    env->SetFogColorOverride(enabled);  
}

bool Environment::GetFogColorOverride() 
{
    OgreRenderer::EC_OgreEnvironment* env = GetEnvironmentComponent();
    if (!env)
        return false;
         
    return env->GetFogColorOverride();
}

QVector<float> Environment::GetFogGroundColor()
{
    OgreRenderer::EC_OgreEnvironment* env = GetEnvironmentComponent();
    if (!env)
        return QVector<float>();

    Ogre::ColourValue color = env->GetGroundFogColor();     
    QVector<float> vec; 
    vec<<color[0]<<color[1]<<color[2];
    return vec;
}

QVector<float> Environment::GetFogWaterColor()
{
    OgreRenderer::EC_OgreEnvironment* env = GetEnvironmentComponent();
    if (!env)
        return QVector<float>();
        
    Ogre::ColourValue color = env->GetWaterFogColor();     
    QVector<float> vec; 
    vec<<color[0]<<color[1]<<color[2];
    return vec;
}

void Environment::Update(f64 frametime)
{
    OgreRenderer::EC_OgreEnvironment* env = GetEnvironmentComponent();
    if (!env)
        return;
         
    env->UpdateVisualEffects(frametime);
}

bool Environment::IsCaelum()
{
    OgreRenderer::EC_OgreEnvironment* env = GetEnvironmentComponent();
    if (!env)
        return false;
        
    return env->IsCaelumUsed();
}

void Environment::SetGroundFogColor(const QVector<float>& color)
{
    OgreRenderer::EC_OgreEnvironment* env = GetEnvironmentComponent();
    if (!env)
        return;

    Ogre::ColourValue fogColour(color[0], color[1], color[2]);
    env->SetGroundFogColor(fogColour); 
}
        
void Environment::SetWaterFogColor(const QVector<float>& color)
{
    OgreRenderer::EC_OgreEnvironment* env = GetEnvironmentComponent();
    if (!env)
        return;

    Ogre::ColourValue fogColour(color[0], color[1], color[2]);
    env->SetWaterFogColor(fogColour); 
}

void Environment::SetGroundFogDistance(float fogStart, float fogEnd)
{
    OgreRenderer::EC_OgreEnvironment* env = GetEnvironmentComponent();
    if (!env)
        return;
        
    env->SetGroundFogStart(fogStart);
    env->SetGroundFogEnd(fogEnd);
}

void Environment::SetWaterFogDistance(float fogStart, float fogEnd)
{
    OgreRenderer::EC_OgreEnvironment* env = GetEnvironmentComponent();
    if (!env)
        return;
        
    env->SetWaterFogStart(fogStart);
    env->SetWaterFogEnd(fogEnd);
}

float Environment::GetWaterFogStartDistance()
{
    OgreRenderer::EC_OgreEnvironment* env = GetEnvironmentComponent();
    if (!env)
        return 0.0;
        
    return env->GetWaterFogStart();
}

float Environment::GetWaterFogEndDistance()
{
    OgreRenderer::EC_OgreEnvironment* env = GetEnvironmentComponent();
    if (!env)
        return 0.0;
    return env->GetWaterFogEnd();
}

float Environment::GetGroundFogStartDistance()
{
    OgreRenderer::EC_OgreEnvironment* env = GetEnvironmentComponent();
    if (!env)
        return 0.0;
        
    return env->GetGroundFogStart();
}

float Environment::GetGroundFogEndDistance()
{
    OgreRenderer::EC_OgreEnvironment* env = GetEnvironmentComponent();
    if (!env)
        return 0.0;
        
    return env->GetGroundFogEnd();
}

void Environment::SetSunDirection(const QVector<float>& vector)
{
    OgreRenderer::EC_OgreEnvironment* env = GetEnvironmentComponent();
    if (!env)
        return;
    // Assure that we do not given too near of zero vector values, a la HACK. 
    float squaredLength = vector[0] * vector[0] + vector[1]* vector[1] + vector[2] * vector[2];
    // Length must be diffrent then zero, so we say that value must be higher then our tolerance. 
    float tolerance = 0.001;
    
    if ( squaredLength > tolerance) 
       env->SetSunDirection(Vector3df(vector[0], vector[1], vector[2]));
}

QVector<float> Environment::GetSunDirection() 
{
    OgreRenderer::EC_OgreEnvironment* env = GetEnvironmentComponent();
    if (!env)
        return QVector<float>(3);

    Vector3df vec = env->GetSunDirection();
    QVector<float> vector;
    vector.append(vec.x), vector.append(vec.y), vector.append(vec.z);
    return vector;
}

void Environment::SetSunColor(const QVector<float>& vector)
{
    OgreRenderer::EC_OgreEnvironment* env = GetEnvironmentComponent();
    if (!env)
        return;
    
    
    if ( vector.size() == 4)
    {
        Color color(vector[0], vector[1], vector[2], vector[3]);
        env->SetSunColor(color);
    }
    else if ( vector.size() == 3 )
    {
        Color color(vector[0], vector[1], vector[2], 1.0);
        env->SetSunColor(color);
    }
  
}
     
QVector<float> Environment::GetSunColor()
{
    OgreRenderer::EC_OgreEnvironment* env = GetEnvironmentComponent();
    if (!env)
        return QVector<float>(4);
        
    Color color = env->GetSunColor();
    QVector<float> vec(4);
    vec[0] = color.r, vec[1] = color.g, vec[2] = color.b, vec[3] = color.a;
    return vec;
}

QVector<float> Environment::GetAmbientLight()
{
    OgreRenderer::EC_OgreEnvironment* env = GetEnvironmentComponent();
    if (!env)
        return QVector<float>(3);
        
   Color color = env->GetAmbientLightColor(); 
   QVector<float> vec(3);
   vec[0] = color.r, vec[1] = color.g, vec[2] = color.b;
   return vec;
}

void Environment::SetAmbientLight(const QVector<float>& vector)
{
    OgreRenderer::EC_OgreEnvironment* env = GetEnvironmentComponent();
    if (!env)
        return;

    env->SetAmbientLightColor(Color(vector[0], vector[1], vector[2]));
}
     

}

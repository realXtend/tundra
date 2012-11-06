// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "AvatarDescHelpers.h"
#include "AssetAPI.h"
#include "MemoryLeakCheck.h"

void AppearanceModifier::ResetAccumulation()
{
    sum_ = 0.0f;
    samples_ = 0;
}

void AppearanceModifier::AccumulateValue(float value, bool use_average)
{
    sum_ += value;
    samples_++;

    if (manual_)
        return;
    
    if (!use_average)
    {
        value_ = sum_;
        if (value_ < 0.0f) value_ = 0.0f;
        if (value_ > 1.0f) value_ = 1.0f;
    }
    else
    {
        value_ = sum_ / samples_;
        if (value_ < 0.0f) value_ = 0.0f;
        if (value_ > 1.0f) value_ = 1.0f;
    }
}

float SlaveModifier::GetMappedValue(float master_value)
{
    // If no positions to interpolate, map master slider directly to modifier pos
    if (mapping_.size() < 2)
    {
        return master_value;
    }

    // Find out the minimum/maximum range of supported master positions
    float min_value = 1.0f;
    float max_value = 0.0f;
    int i;
    
    for (i = 0; i < (int)mapping_.size(); ++i)
    {
        if (mapping_[i].master_ < min_value)
            min_value = mapping_[i].master_;
        if (mapping_[i].master_ > max_value)
            max_value = mapping_[i].master_;
    }

    // Now cap the master position according to what is supported
    if (master_value < min_value)
        master_value = min_value;
    if (master_value > max_value)
        master_value = max_value;
    
    // Find beginning pos. of interpolation
    for (i = (int)mapping_.size()-1; i >= 0; --i)
    {
        if (mapping_[i].master_ <= master_value)
            break;
    }

    // If at the endpoint, simply return the value at end
    if (i == (int)mapping_.size()-1)
    {
        return mapping_[i].slave_;
    }

    float delta = mapping_[i+1].slave_ - mapping_[i].slave_;
    float master_delta = mapping_[i+1].master_ - mapping_[i].master_;
    float weight = 0.0f;
    if (master_delta > 0.0f)
    {
        weight = (master_value - mapping_[i].master_) / master_delta;
    }

    return mapping_[i].slave_ + weight * delta;
}


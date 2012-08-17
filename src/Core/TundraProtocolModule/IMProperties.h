/**
 *  Copyright (c) 2011 CIE / University of Oulu, All Rights Reserved
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 */
 
#pragma once

#include "Types.h"

class IMProperties
{

private:

    bool enabled_;
    bool euclidean_mode_;
    bool relevance_mode_;
    bool raycast_mode_;
    double max_range_;
    double critical_range_;
    double raycast_range_;
    u32 update_interval_;

public:

    IMProperties();

    ~IMProperties();

    bool GetEuclideanMode();
    bool GetRelevanceMode();
    bool GetRaycastMode();
    double GetMaxRange();
    double GetCriticalRange();
    u32 GetUpdateInterval();
    double GetRaycastRange();

    void SetEuclideanMode(bool mode);
    void SetRelevanceMode(bool mode);
    void SetRaycastMode(bool mode);
    void SetMaximumRange(double r);
    void SetCriticalRange(double r);
    void SetUpdateInterval(u32 i);
    void SetRaycastRange(double r);

    bool isEnabled();
    void setEnabled(bool mode);

    void printConfiguration();
};

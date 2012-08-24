// For conditions of distribution and use, see copyright notice in LICENSE

#include "IMProperties.h"
#include "LoggingFunctions.h"

IMProperties::IMProperties()
{
    enabled_ = false;
    relevance_mode_ = false;
    raycast_mode_ = false;
    euclidean_mode_ = false;
    max_range_ = 60;
    critical_range_ = 20;
    raycast_range_ = 20;
    update_interval_ = 1000000000;
}

IMProperties::~IMProperties()
{

}

bool IMProperties::isEuclideanMode()              { return euclidean_mode_; }
bool IMProperties::isRelevanceMode()              { return relevance_mode_; }
bool IMProperties::isRaycastMode()                { return raycast_mode_;   }
double IMProperties::getRaycastRange()            { return raycast_range_;  }
double IMProperties::getMaxRange()                { return max_range_;      }
double IMProperties::getCriticalRange()           { return critical_range_; }
u32 IMProperties::getUpdateInterval()             { return update_interval_;}

void IMProperties::setEuclideanMode(bool mode)    { euclidean_mode_ = mode; }
void IMProperties::setRelevanceMode(bool mode)    { relevance_mode_ = mode; }
void IMProperties::setRaycastMode(bool mode)      { raycast_mode_ = mode;   }
void IMProperties::setMaximumRange(double r)      { max_range_ = r;         }
void IMProperties::setCriticalRange(double r)     { critical_range_ = r;    }
void IMProperties::setUpdateInterval(u32 i)       { update_interval_ = i;   }
void IMProperties::setRaycastRange(double r)      { raycast_range_ = r;     }

bool IMProperties::isEnabled()                    { return enabled_;        }
void IMProperties::setEnabled(bool mode)          { enabled_ = mode;        }

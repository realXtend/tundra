/**
 *  Copyright (c) 2011 CIE / University of Oulu, All Rights Reserved
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 */
 
#pragma once

#include "Types.h"

/// The IMProperties class
/** A class that stores the configuration
    of the Interest Manager. */
class IMProperties
{

private:

    /// Variables that store the IM settings.
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

    /// Returns a boolean to describe if the Euclidean distance filter is enabled or not.
    /** @return boolean - status of the euclidean distance filter. */
    bool isEuclideanMode();

    /// Returns a boolean to describe if the Relevance filter is enabled or not.
    /** @return boolean - status of the relevance filter. */
    bool isRelevanceMode();

    /// Returns a boolean to describe if the Ray Visibility filter is enabled or not.
    /** @return boolean - status of the ray visibility filter. */
    bool isRaycastMode();

    /// Returns a boolean to describe if the Interest Manager Scheme is enabled or not.
    /** @return boolean - status of the IM scheme. */
    bool isEnabled();

    /// Returns the maximum cutoff range.
    /** @return double - the cutoff range for the filters. */
    double getMaxRange();

    /// Returns the radius of the critical area.
    /** @return double - the critical range for the filters. */
    double getCriticalRange();

    /// Returns the raycasting range.
    /** @return double - the cutoff range for the ray visibility filter*/
    double getRaycastRange();

    /// Returns the maximum update interval of the relevance filter.
    /** @return u32 - time telling the maximum update interval in nanoseconds*/
    u32 getUpdateInterval();

    /// Enables or disables the euclidean distance filter
    /** @param bool mode that tells the new status for the filter. */
    void setEuclideanMode(bool mode);

    /// Enables or disables the relevance filter
    /** @param bool mode that tells the new status for the filter. */
    void setRelevanceMode(bool mode);

    /// Enables or disables the ray visibility filter
    /** @param bool mode that tells the new status for the filter. */
    void setRaycastMode(bool mode);

    /// Sets the cutoff range.
    /** @param bool mode that tells the new status for the filter. */
    void setMaximumRange(double r);

    /// Sets the critical range.
    /** @param bool mode that tells the new status for the filter. */
    void setCriticalRange(double r);

    /// Sets the update interval of the relevance filter.
    /** @param bool mode that tells the new status for the filter. */
    void setUpdateInterval(u32 i);

    /// Sets the raycasting cutoff range.
    /** @param bool mode that tells the new status for the filter. */
    void setRaycastRange(double r);

    /// Enables or disables the IM scheme.
    /** @param bool mode that tells the new status for the filter. */
    void setEnabled(bool mode);
};

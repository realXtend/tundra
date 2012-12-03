// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "TundraCoreApi.h"

#include <exception>
#include <cstring>

/// Generic exception class.
/** Can be used to throw generic exceptions.
    @note Uses non-safe string manipulation, so do not supply exception parameters from outside source! */
class TUNDRACORE_API Exception : public std::exception
{
public:
    /// default constructor
    Exception() : std::exception(), what_(0), do_free_(0) {}

    /// constructor that takes a string
    explicit Exception(const char * const& what) : std::exception()
    {
        do_free_ = 1;

        size_t len = strlen(what);
        what_ = static_cast< char* >(malloc(len + 1));
        strcpy(const_cast< char* >(what_), what);
    }

    /// copy constructor
    Exception(const Exception &other) : std::exception(other)
    {
        do_free_ = other.do_free_;

        if (do_free_)
        {
            size_t len = strlen(other.what_);
            what_ = static_cast< char* >(malloc(len + 1));
            strcpy(const_cast< char* >(what_), other.what_);
        } else
        {
            what_ = other.what_;
        }
    }

    /// assignment operator
    Exception& operator =(const Exception &other)
    {
        if (this != &other)
        {
            do_free_ = other.do_free_;

            if (do_free_)
            {
                size_t len = strlen(other.what_);
                what_ = static_cast< char* >(malloc(len + 1));
                strcpy(const_cast< char* >(what_), other.what_);
            } else
            {
                what_ = other.what_;
            }
        }

        return *this;
    }

    /// destructor
    virtual ~Exception() throw ()
    {
        if (do_free_)
        {
            free(const_cast< char* >(what_));
        }
    }

    /// returns the exception string
    virtual const char *what () const throw ()
    {
        if (what_ != 0)
            return what_;
        else
            return "Unknown exception";
    }
private:
    const char *what_;
    int do_free_;
};


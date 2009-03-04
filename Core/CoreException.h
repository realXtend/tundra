// For conditions of distribution and use, see copyright notice in license.txt

#ifndef __incl_Core_Exception_h__
#define __incl_Core_Exception_h__

namespace Core
{
    class Exception : public std::exception
    {
    public:
        Exception() : std::exception() {}

        explicit Exception(const char *exception) : std::exception(), exception_(exception) {}

        Exception(const Exception &other)
        {
            exception_ = other.exception_;
        }

        virtual ~Exception() {}

        virtual const char *what () const throw ()
        {
            return exception_;
        }
    private:
        const char *exception_;
    };
}
#endif



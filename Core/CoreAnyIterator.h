// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Core_AnyIterator_h
#define incl_Core_AnyIterator_h

namespace Core
{
    //! Interface for virtual iterator adaptor
    /*! Abstract iterator that can be used on interface
        classes without knowledge of the exact type
        of iterator a derived class uses.
    */
    template <class base_type>
    class MODULE_API AnyIterator_Impl_Abstract
    {
    public:
        AnyIterator_Impl_Abstract() {}
        virtual ~AnyIterator_Impl_Abstract() {}

        virtual bool operator <(const AnyIterator_Impl_Abstract &rhs) const = 0;
        virtual bool operator ==(const AnyIterator_Impl_Abstract &rhs) const = 0;

        virtual AnyIterator_Impl_Abstract &operator ++() = 0;

        virtual base_type &operator *() = 0;
    };

    //! Inherit to create your own iterator adaptor
    template <class base_iterator, class base_type>
    class MODULE_API AnyIterator_Impl : public AnyIterator_Impl_Abstract<base_type>
    {
    public:
        AnyIterator_Impl(base_iterator iter) : iter_(iter) {}
        virtual ~AnyIterator_Impl() {}

        virtual bool operator <(const AnyIterator_Impl_Abstract &rhs) const { return iter_ != dynamic_cast<const AnyIterator_Impl&>(rhs).iter_; }
        virtual bool operator ==(const AnyIterator_Impl_Abstract &rhs) const { return iter_ == dynamic_cast<const AnyIterator_Impl&>(rhs).iter_; }

        virtual AnyIterator_Impl &operator ++() { ++iter_; return *this; }

    protected:
        base_iterator iter_;
    };

    template <class base_type>
    class AnyIterator
    {
    public:
        AnyIterator(boost::shared_ptr< AnyIterator_Impl_Abstract<base_type> > impl) : impl_(impl) {}
        ~AnyIterator() {}

        bool operator <(const AnyIterator &rhs) const { return *impl_ < *rhs.impl_; }
        bool operator ==(const AnyIterator &rhs) const { return *impl_ == *rhs.impl_; }
        bool operator !=(const AnyIterator &rhs) const { return !(*this == rhs); } 
        bool operator <=(const AnyIterator &rhs) const { return *this < rhs || *this == rhs; }
        bool operator >(const AnyIterator &rhs) const { return !(*this <= rhs); }

        AnyIterator &operator ++() { ++(*impl_); return *this; }

        base_type &operator *() { assert(impl_); return **impl_; }
    private:
        boost::shared_ptr< AnyIterator_Impl_Abstract<base_type> > impl_;
    };
}
#endif


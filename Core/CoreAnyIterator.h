// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Core_AnyIterator_h
#define incl_Core_AnyIterator_h

//! Interface for virtual forward iterator adaptor
/*! Abstract iterator that can be used on interface
    classes without knowledge of the exact type
    of iterator a derived class uses. */
template <class base_type>
class AnyIteratorInterface
{
public:
    AnyIteratorInterface() {}
    virtual ~AnyIteratorInterface() {}

    virtual AnyIteratorInterface &operator =(const AnyIteratorInterface &rhs) = 0;
    virtual bool operator ==(const AnyIteratorInterface &rhs) const = 0;
    virtual bool operator !=(const AnyIteratorInterface &rhs) const = 0;

    virtual AnyIteratorInterface &operator ++() = 0;

    virtual base_type &operator *() = 0;
};

//! Inherit to create your own iterator adaptor
template <class base_iterator, class base_type>
class AnyIteratorImpl : public AnyIteratorInterface<base_type>
{
public:
    AnyIteratorImpl(base_iterator iter) : iter_(iter) {}
    AnyIteratorImpl(const AnyIteratorInterface<base_type> &rhs) : iter_(dynamic_cast<const AnyIteratorImpl&>(rhs).iter_) {}
    virtual ~AnyIteratorImpl() {}

    virtual AnyIteratorInterface<base_type> &operator =(const AnyIteratorInterface<base_type> &rhs)
    { 
        if (this != &rhs)
            iter_ = dynamic_cast<const AnyIteratorImpl&>(rhs).iter_;
        return *this;
    }
    virtual bool operator ==(const AnyIteratorInterface<base_type> &rhs) const { return iter_ == static_cast<const AnyIteratorImpl&>(rhs).iter_; }
    virtual bool operator !=(const AnyIteratorInterface<base_type> &rhs) const { return iter_ != static_cast<const AnyIteratorImpl&>(rhs).iter_; }

    virtual AnyIteratorImpl &operator ++() { ++iter_; return *this; }

protected:
    base_iterator iter_;
};

//! Adaptor for list, set and vector iterators
template <class base_iterator, class base_type>
class ListIterator : public AnyIteratorImpl<base_iterator, base_type>
{
    ListIterator();
public:
    ListIterator(base_iterator iter) : AnyIteratorImpl<base_iterator, base_type> (iter) {}
    virtual ~ListIterator() {}

    virtual base_type &operator *() { return *AnyIteratorImpl<base_iterator, base_type>::iter_; }
};

//! Adaptor for map iterator
template <class base_iterator, class base_type>
class MapIterator : public AnyIteratorImpl<base_iterator, base_type>
{
    MapIterator();
public:
    MapIterator(base_iterator iter) : AnyIteratorImpl<base_iterator, base_type> (iter) {}
    virtual ~MapIterator() {}

    virtual base_type &operator *() { return AnyIteratorImpl<base_iterator, base_type>::iter_->second; }
};

//! The concrete iterator object used by clients to iterate over a container of base_type objects.
template <class base_type>
class AnyIterator
{
public:
    AnyIterator(boost::shared_ptr< AnyIteratorInterface<base_type> > impl) : impl_(impl) {}
    ~AnyIterator() {}

    bool operator ==(const AnyIterator &rhs) const { return *impl_ == *rhs.impl_; }
    bool operator !=(const AnyIterator &rhs) const { return !(*this == rhs); }

    AnyIterator &operator ++() { ++(*impl_); return *this; }

    base_type &operator *() { assert(impl_); return **impl_; }
private:
    boost::shared_ptr< AnyIteratorInterface<base_type> > impl_;
};

#endif


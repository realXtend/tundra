// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_IComponentFactory_h
#define incl_Interfaces_IComponentFactory_h

#include <boost/shared_ptr.hpp>

class IComponent;

//! A class implements this interface to provide factory functionality for generating one type of entity-components.
/*! Each EC has its own factory for creating the component.

    \note When creating new entity components, it is not necessary to use this class if DECLARE_EC -macro is used.
*/
class IComponentFactory
{
public:
    IComponentFactory() {}
    virtual ~IComponentFactory() {}

    virtual boost::shared_ptr<IComponent> operator()() = 0;
    virtual boost::shared_ptr<IComponent> operator()(const boost::shared_ptr<IComponent> &) = 0;
};

#endif

// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_ComponentInterface_h
#define incl_Interfaces_ComponentInterface_h

#include "CoreDefines.h"
#include "ComponentFactoryInterface.h"
#include "ComponentRegistrarInterface.h"

#include <QObject>

namespace Foundation
{
    class Framework;

    //! The abstract topmost level interface common for all components. Do not inherit from this when creating new components.
    /*! The only common thing components currently have is they each have a name.
        Otherwise each component contains their own data and the data should be accessed via
        an upcast from this base class.

        \ingroup Scene_group
    */
/*
    class MODULE_API ComponentInterfaceAbstract
    {
    public:
        ComponentInterfaceAbstract()  {}
        virtual ~ComponentInterfaceAbstract() { }
        
        virtual const std::string &Name() const = 0;
    };
*/
    //! Base class for all components. Inherit from this class when creating new components.
    /*! Use the ComponentInterface typedef to refer to the abstract component type.
    */
    class MODULE_API ComponentInterface : public QObject
    {
        Q_OBJECT

    public:
        explicit ComponentInterface(const Foundation::Framework *framework);
        ComponentInterface(const ComponentInterface &rhs);
        virtual ~ComponentInterface();
        virtual const std::string &Name() const = 0;

    protected:
        ComponentInterface();

    private:
        const Foundation::Framework * const framework_;
    };
}

#endif

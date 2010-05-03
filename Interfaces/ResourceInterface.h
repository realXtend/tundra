// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_ResourceInterface_h
#define incl_Interfaces_ResourceInterface_h

// Disable C4251 warnings in MSVC.
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 )
#endif

#include "CoreTypes.h"
#include "EventDataInterface.h"
#include "CoreModuleApi.h"

#include <map>
#include <string>

namespace Foundation
{
    class ResourceInterface;
    typedef boost::shared_ptr<ResourceInterface> ResourcePtr;
    typedef boost::weak_ptr<ResourceInterface> ResourceWeakPtr;
    typedef std::map<std::string, Foundation::ResourcePtr> ResourceMap;

    //! Reference to a resource depended on
    struct ResourceReference
    {
        ResourceReference(const std::string& id, const std::string& type) :
            id_(id),
            type_(type)
        {
        }
        
        //! Resource id
        std::string id_;
        //! Resource type
        std::string type_;
    };
    
    typedef std::vector<ResourceReference> ResourceReferenceVector;
        
    //! An identifiable resource object of some kind. Subclass as needed.
    /*! Note that resources are different from assets in the sense that assets are binary blobs of data, while
        resources should be usable, for example an image which stores its dimensions and actual decoded image data
     */
    class MODULE_API ResourceInterface
    {
    public:
        //! default constructor
        ResourceInterface() {}

        //! constructor
        /*! \param id identifier of resource
         */
        ResourceInterface(const std::string& id) : id_(id) {}
        
        //! destructor
        virtual ~ResourceInterface() {}
        
        //! sets identifier
        /*! \param id new identifier
         */
        void SetId(const std::string& id) { id_ = id; }

        //! returns whether content is valid and usable
        /*! mainly for subsystem internal use: invalid resources should not be returned to caller
         */
        virtual bool IsValid() const = 0;
        
        //! returns identifier
        const std::string& GetId() const { return id_; }

        //! returns non-const reference vector
        ResourceReferenceVector& GetReferences() { return references_; }

        //! returns type in text form
        virtual const std::string& GetType() const = 0;

    protected:
        //! resource identifier
        std::string id_;
        
        //! content valid-flag
        bool valid_;

        //! references to other resources this resource depends on
        ResourceReferenceVector references_;
    };
}

//! resource events
namespace Resource
{
    namespace Events
    {
        //! Sent when a resource is ready. Uses the event data structure ResourceReady.
        static const event_id_t RESOURCE_READY = 1;
        
        //! Sent when a resource request could not be served. Uses the event data structure ResourceCanceled.
        static const event_id_t RESOURCE_CANCELED = 2;
        
        //! Resource ready event data
        class ResourceReady : public Foundation::EventDataInterface
        {
        public:
            ResourceReady(const std::string& id, Foundation::ResourcePtr resource, request_tag_t tag) :
                id_(id),
                resource_(resource),
                tag_(tag)
            {
            }
            
            virtual ~ResourceReady()
            {
            }
            
            //! ID of resource that is ready
            std::string id_;
            //! Pointer to ready resource
            Foundation::ResourcePtr resource_;
            //! Request tag identifying the resource request
            request_tag_t tag_;
        };
        
        //! Resource canceled event data
        class ResourceCanceled : public Foundation::EventDataInterface
        {
        public:
            ResourceCanceled(const std::string& id, request_tag_t tag) :
                id_(id),
                tag_(tag)
            {
            }
            
            virtual ~ResourceCanceled()
            {
            }
            
            //! ID of resource that was canceled
            std::string id_;
            //! Request tag identifying the resource request
            request_tag_t tag_;
        };
    }
}

#ifdef _MSC_VER
#pragma warning( pop )
#endif

#endif

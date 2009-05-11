// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_ResourceInterface_h
#define incl_Interfaces_ResourceInterface_h

#include "EventDataInterface.h"

namespace Foundation
{
    class ResourceInterface;    
    typedef boost::shared_ptr<ResourceInterface> ResourcePtr;
    typedef std::map<std::string, Foundation::ResourcePtr> ResourceMap;

    //! An identifiable resource object of some kind. Subclass as needed.
    /*! Note that resources are different from assets in the sense that assets are binary blobs of data, while
        resources should be usable, for example an image which stores its dimensions and actual decoded image data
     */
    class MODULE_API ResourceInterface
    {
    public:
        typedef std::vector<std::string> ReferenceVector;
        
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

        //! returns identifier
        const std::string& GetId() const { return id_; }

        //! returns non-const reference vector
        ReferenceVector& GetReferences() { return references_; }

        //! returns type in text form
        virtual const std::string& GetType() const = 0;

    protected:
        //! resource identifier
        std::string id_;

        //! references to other resources (resource id's)
        ReferenceVector references_;
    };
}

//! resource events
namespace Resource
{
    namespace Events
    {
        //! resource ready event id. When this is posted, the data passed along the event is of type Resource::Events::ResourceReady.
        static const Core::event_id_t RESOURCE_READY = 1;
        
        //! resource ready event data
        class ResourceReady : public Foundation::EventDataInterface
        {
        public:
            ResourceReady(const std::string& id, Foundation::ResourcePtr resource, Core::request_tag_t tag) :
                id_(id),
                resource_(resource),
                tag_(tag)
            {
            }
            
            virtual ~ResourceReady()
            {
            }
        
            std::string id_;
            Foundation::ResourcePtr resource_;
            Core::request_tag_t tag_;
        };
    }
}

#endif
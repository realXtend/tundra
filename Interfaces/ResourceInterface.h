// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_ResourceInterface_h
#define incl_Interfaces_ResourceInterface_h

namespace Foundation
{
    class ResourceInterface;    
    typedef boost::shared_ptr<ResourceInterface> ResourcePtr;

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
        
        //! sets identified
        /*! \param id new identifier
         */
        void SetId(const std::string& id) { id_ = id; }

        //! returns identifier
        const std::string& GetId() { return id_; }

        //! returns type in text form
        virtual const std::string& GetTypeName() = 0;

    protected:
        //! resource identifier
        std::string id_;
    };
}

#endif
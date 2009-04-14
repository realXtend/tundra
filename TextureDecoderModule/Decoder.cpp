// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "Decoder.h"

#include <openjpeg.h>

namespace TextureDecoder
{

    Decoder::Decoder(Foundation::Framework* framework) : 
        framework_(framework),
        asset_service_(NULL)
    {
        Foundation::ServiceManagerPtr service_manager = framework_->GetServiceManager();
        
        if (service_manager->IsRegistered(Foundation::Service::ST_Asset))
            asset_service_ = service_manager->GetService<Foundation::AssetServiceInterface>(Foundation::Service::ST_Asset);
    }
    
    Decoder::~Decoder()
    {
    }

    //void Decode()
    //{
    //    opj_dparameters_t parameters;
    //    opj_set_default_decoder_parameters(&parameters);
    //}
}


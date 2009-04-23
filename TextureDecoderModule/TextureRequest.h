// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_TextureDecoder_TextureRequest_h
#define incl_TextureDecoder_TextureRequest_h

namespace TextureDecoder
{
    //! an ongoing texture request, used internally by TextureService.
    class TextureRequest
    {
    public:
        //! constructor
        TextureRequest();
        
        //! constructor
        /*! \param id asset id
         */
        TextureRequest(const std::string& id);
        
        //! destructor
        ~TextureRequest();

        //! sets request status
        void SetRequested(bool requested) { requested_ = requested; }

        //! sets decode request status
        void SetDecodeRequested(bool requested) { decode_requested_ = requested; }

        //! updates size & received count
        /*! \param size total size of asset (from asset service)
            \param received received continuous bytes (from asset service)
         */
        void UpdateSizeReceived(Core::uint size, Core::uint received);

        //! sets dimensions & component count (when known)
        /*! \param width texture width
            \param height texture height
            \param components texture components
         */
        void SetSize(Core::uint width, Core::uint height, Core::uint components);

        //! sets amount of quality levels (when known)
        /*! \param levels amount of quality levels
         */
        void SetLevels(int levels);

        //! sets next level to be decoded
        void SetNextLevelToDecode();
        
        //! sets current level = nextz
        void DecodeSuccess();

        //! checks if enough data to decode next level
        bool HasEnoughData();

        //! estimates needed data size for a given level
        /*! \param level quality level
         */
        Core::uint EstimateDataSize(int level);

        //! returns asset id
        const std::string& GetId() { return id_; }

        //! returns request status
        bool IsRequested() { return requested_; }

        //! returns decode request status
        bool IsDecodeRequested() { return decode_requested_; }

        //! returns total data size, 0 if unknown
        Core::uint GetSize() { return size_; }
        
        //! returns received bytes
        Core::uint GetReceived() { return received_; }

        //! returns width, 0 if unknown
        Core::uint GetWidth() { return width_; }

        //! returns height, 0 if unknown
        Core::uint GetHeight() { return height_; }

        //! returns components, 0 if unknown
        Core::uint GetComponents() { return components_; }

        //! returns amount quality levels, -1 if unknown
        int GetLevels() { return levels_; }
        
        //! returns last decoded level, -1 if none so far
        int GetDecodedLevel() { return decoded_level_; }

        //! returns next level to decode
        int GetNextLevel() { return next_level_; }

    private:
        //! asset on which this request is based
        std::string id_;

        //! whether asset request has been queued
        bool requested_;

        //! whether decode request has been queued
        bool decode_requested_;

        //! total data size, 0 if unknown
        Core::uint size_;

        //! last checked size of download progress (continuous received bytes)
        Core::uint received_;

        //! texture original width, 0 if unknown
        Core::uint width_;

        //! texture original height, 0 if unknown
        Core::uint height_;

        //! components in image, 0 if unknown
        Core::uint components_;

        //! number of quality levels, -1 if unknown
        int levels_;

        //! last decoded quality level, 0 = full quality, -1 if none decoded so far
        int decoded_level_;

        //! next quality level to decode
        int next_level_;
    };
}
#endif
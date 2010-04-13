// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_MumbleVoipModule_Channel_h
#define incl_MumbleVoipModule_Channel_h

namespace MumbleClient
{
    class Channel;
}

namespace MumbleVoip
{
    //! Channel on Mumble server
    //!
    class Channel
    {
    public:
        Channel(const MumbleClient::Channel& c);
        QString Name();
        int Id();
        QString Description();
    private:
        const MumbleClient::Channel& channel_;
    };

}// namespace MumbleVoip

#endif // incl_MumbleVoipModule_Channel_h

#ifndef incl_Comm_VoipSession_h
#define incl_Comm_VoipSession_h

#include "StableHeaders.h"
#include "Foundation.h"

#include <QObject>
#include <TelepathyQt4/StreamedMediaChannel>
#include "Communication.h"

namespace TpQt4Communication
{
	/**
	EVENTS: 
		- ParticipantLeft
		- ParticipantJoined
		- TextMessageReceived
		- SessionClosed
	*/
	class VoipSession
	{
	public:
		void Invite(Address a);
		void Close();
		void GetParticipants();
		void SetMicrophoneVolume(double value);
		double GetMicrophoneVolume();
	private:
		Tp::StreamedMediaChannelPtr tp_streamed_media_channel_;
		//Tp::FarsightChannel tp_farsight_channel_;
	};



} // end of namaspace: TpQt4Communication

#endif // incl_Comm_ChatSession_h

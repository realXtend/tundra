#ifndef MumbleVoipModule_LinkedMem_h
#define MumbleVoipModule_LinkedMem_h

namespace MumbleVoip
{
	/**
	 * @see http://mumble.sourceforge.net/Link
	 *
	 */
	struct LinkedMem
	{
	#ifdef WIN32
		UINT32	uiVersion;
		DWORD	uiTick;
	#else
		uint32_t uiVersion;
		uint32_t uiTick;
	#endif
		float	fAvatarPosition[3];
		float	fAvatarFront[3];
		float	fAvatarTop[3];
		wchar_t	name[256];
		float	fCameraPosition[3];
		float	fCameraFront[3];
		float	fCameraTop[3];
		wchar_t	identity[256];
	#ifdef WIN32
		UINT32	context_len;
	#else
		uint32_t context_len;
	#endif
		unsigned char context[256];
		wchar_t description[2048];
	};

} // end of namespace: MumbleVoip

#endif // MumbleVoipModule_LinkPlugin_h
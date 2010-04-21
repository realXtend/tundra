#include "StableHeaders.h"
#include "LinkPlugin.h"
#include "LinkedMem.h"

#ifdef Q_WS_X11
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/fcntl.h>
#endif

//! QString::ToStdWString() doesn't work with Visual Studio 2008
std::wstring QStringToStdWString(QString &s)
{
    std::string temp1 = s.toStdString();
    std::wstring temp2(temp1.size(), L' ');
    std::copy(temp1.begin(), temp1.end(), temp2.begin());
    return temp2;
}

namespace MumbleVoip
{
	LinkPlugin::LinkPlugin(): linked_mem_(0), send_data_(false)
	{
	}

    void LinkPlugin::Start()
    {
        if (linked_mem_ == 0)
            InitializeLinkedMem();
        if (linked_mem_ == 0)
            return;
        send_data_ = true;
    }

    void LinkPlugin::Stop()
    {
        send_data_ = false;
    }

    bool LinkPlugin::IsRunning() const
    {
        if (linked_mem_ != 0 && send_data_)
            return true;
        else
            return false;
    }

    QString LinkPlugin::GetReason() const
    {
        return reason_;
    }

	LinkPlugin::~LinkPlugin()
	{
	}

	void LinkPlugin::SendData()
	{
        if (!send_data_)
            return;

		if (! linked_mem_)
			return;

		if(linked_mem_->uiVersion != 2)
		{
            wcsncpy(linked_mem_->name, QStringToStdWString(application_name_).c_str(), 256);
            wcsncpy(linked_mem_->description, QStringToStdWString(application_description_).c_str(), 2048);
			linked_mem_->uiVersion = 2;
		}
		linked_mem_->uiTick++;

		linked_mem_->fAvatarPosition[0] = avatar_position_.y;
		linked_mem_->fAvatarPosition[1] = avatar_position_.z;
		linked_mem_->fAvatarPosition[2] = -avatar_position_.x;

		linked_mem_->fAvatarTop[0] = avatar_top_.y;
		linked_mem_->fAvatarTop[1] = avatar_top_.z;
		linked_mem_->fAvatarTop[2] = -avatar_top_.x;

		linked_mem_->fAvatarFront[0] = avatar_front_.y;
		linked_mem_->fAvatarFront[1] = avatar_front_.z;
		linked_mem_->fAvatarFront[2] = -avatar_front_.x;

        linked_mem_->fCameraFront[0] = camera_front_.y;
		linked_mem_->fCameraFront[1] = camera_front_.z;
		linked_mem_->fCameraFront[2] = -camera_front_.x;

		linked_mem_->fCameraTop[0] = camera_top_.y;
		linked_mem_->fCameraTop[1] = camera_top_.z;
		linked_mem_->fCameraTop[2] = -camera_top_.x;

		linked_mem_->fCameraPosition[0] = camera_position_.y;
		linked_mem_->fCameraPosition[1] = camera_position_.z;
		linked_mem_->fCameraPosition[2] = -camera_position_.x;

        // Identifier which uniquely identifies a certain player in a context (e.g. the ingame Name).
        wcsncpy(linked_mem_->identity, QStringToStdWString(user_id_).c_str(), 256);

        // Context should be equal for players which should be able to hear each other positional and
	    // differ for those who shouldn't (e.g. it could contain the server+port and team)
        int len = strlen(context_id.toStdString().c_str());
        memcpy(linked_mem_->context, context_id.toStdString().c_str(), len+1);
	    linked_mem_->context_len = len;
	}

	void LinkPlugin::SetApplicationName(const QString& name)
	{
        application_name_ = name;
        application_name_.truncate(255);
	}
	
	void LinkPlugin::SetUserIdentity(const QString& id)
	{
        user_id_ = id;
        user_id_.truncate(255);
	}

    void LinkPlugin::SetContextId(const QString& id)
    {
        context_id = id;
        context_id.truncate(255);
    }

    void LinkPlugin::SetApplicationDescription(const QString& description)
    {
        application_description_ = description;
        application_description_.truncate(2047);
    }
	
    void LinkPlugin::SetAvatarPosition(Vector3df position, Vector3df front, Vector3df top)
	{
        avatar_position_ = position;
        avatar_top_ = top;
        avatar_front_ = front;
	}
	
	void LinkPlugin::SetCameraPosition(Vector3df position, Vector3df front, Vector3df top)
	{
        camera_position_ = position;
        camera_top_ = top;
        camera_front_ = front;
	}

    void LinkPlugin::InitializeLinkedMem()
    {
#ifdef WIN32
		HANDLE hMapObject = OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, L"MumbleLink");
		if (hMapObject == NULL)
        {
            reason_ = "Shared memory is not available, please ensure that mumble client is running.";
			return;
        }

		linked_mem_ = (LinkedMem *) MapViewOfFile(hMapObject, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(LinkedMem));
		if (linked_mem_ == NULL) {
			CloseHandle(hMapObject);
			hMapObject = NULL;
            reason_ = "Shared memory is not available, cannot open the file.";
			return;
		}
#else
		char memname[256];
		snprintf(memname, 256, "/MumbleLink.%d", getuid());

		int shmfd = shm_open(memname, O_RDWR, S_IRUSR | S_IWUSR);

		if (shmfd < 0)
        {
            reason_ = "Shared memory is not available, please ensure that mumble client is running.";
			return;
		}

		linked_mem_ = (LinkedMem *)(mmap(NULL, sizeof(struct LinkedMem), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd,0));

		if (linked_mem_ == (void *)(-1))
        {
            reason_ = "Shared memory is not available, cannot open the file.";
			linked_mem_ = NULL;
			return;
		}
#endif
    }

} // end of namespace: MumbleVoip

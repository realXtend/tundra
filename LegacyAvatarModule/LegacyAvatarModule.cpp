// For conditions of distribution and use, see copyright notice in license.txt
#include "StableHeaders.h"

#include "LegacyAvatarModule.h"
#include <Poco/ClassLibrary.h>

#include <iomanip>
#include <limits>
#include <boost/math/special_functions/fpclassify.hpp>

#include <QString>
#include <QStringList>
#include <QFileInfo>
#include <QFile>
#include <string.h>

namespace LegacyAvatar
{
	Mesh *LegacyAvatarModule::mesh_eye = 0;
	Mesh *LegacyAvatarModule::mesh_eyelashes = 0;
	Mesh *LegacyAvatarModule::mesh_hair = 0;
	Mesh *LegacyAvatarModule::mesh_head = 0;
	Mesh *LegacyAvatarModule::mesh_lower_body = 0;
	Mesh *LegacyAvatarModule::mesh_skirt = 0;
	Mesh *LegacyAvatarModule::mesh_upper_body = 0;

	typedef struct tagVector2
	{
		float U;
		float V;
	} Vector2;

	typedef struct tagVector3
	{
		float X;
		float Y;
		float Z;
	} Vector3;

	typedef struct tagFace
	{
		short A;
		short B;
		short C;
	} Face;

	typedef struct tagVertex
	{
		Vector3 baseCoords;
		Vector3 baseNormals;
		Vector3 baseBinormals;
		Vector2 texCoords;
	} Vertex;

	typedef struct tagMorphVertex
	{
		unsigned long VertexIndex;
		Vector3 Coords;
		Vector3 Normal;
		Vector3 Binormal;
		Vector2 texCoord;
	} MorphVertex;
	
	typedef struct tagMorph
	{
		char morphName[64];
		long numVertices;
		MorphVertex *Vertices;
	} Morph;

	typedef struct tagRemap
	{
		long Source;
		long Destination;
	} Remap;

	class Mesh
	{
	public:
		QString Name;
		QString Version;
		char hasWeights;
		char hasDetailTexCoords;
		Vector3 position;
		Vector3 rotationAngles;
		char rotationOrder;
		Vector3 Scale;
		unsigned short numVertices;
		Vertex *Vertices;
		Vector2 *detailTexCoords;
		float *Weights;
		unsigned short numFaces;
		Face *Faces;
		unsigned short numSkinJoints;
		QStringList jointNames;
		long numMorphs;
		Morph *Morphs;
		long numRemaps;
		Remap* Remaps;

		bool Load(const char *file);
	};

#define getvar(x) if (source.read((char *)&(x), sizeof(x)) != sizeof(x)) return false

	bool Mesh::Load(const char *file)
	{
		QString fileName(file);

		QFileInfo sourceInfo(fileName);
		QFile source(fileName);

		if (!source.open(QIODevice::ReadOnly))
			return false;

		Name = QString("avatar/") + sourceInfo.baseName();

		char version[25];
		version[24] = '\0';

		if (source.read(version, 24) != 24)
			return false;

		Version = QString(version);

		getvar(hasWeights);
		getvar(hasDetailTexCoords);

		getvar(position);
		getvar(rotationAngles);
		getvar(rotationOrder);
		getvar(Scale);
		getvar(numVertices);

		Vertices = (Vertex *)malloc(sizeof(Vertex) * numVertices);
		Weights = (float *)calloc(sizeof(float), numVertices);
		detailTexCoords = (Vector2 *)calloc(sizeof(Vector2), numVertices);

		int i;

		for (i = 0 ; i < numVertices ; i++)
			getvar(Vertices[i].baseCoords);

		for (i = 0 ; i < numVertices ; i++)
			getvar(Vertices[i].baseNormals);

		for (i = 0 ; i < numVertices ; i++)
			getvar(Vertices[i].baseBinormals);

		for (i = 0 ; i < numVertices ; i++)
			getvar(Vertices[i].texCoords);

		if (hasDetailTexCoords)
		{
			for (i = 0 ; i < numVertices ; i++)
				getvar(detailTexCoords[i]);
		}

		if (hasWeights)
		{
			for (i = 0 ; i < numVertices ; i++)
				getvar(Weights[i]);
		}

		getvar(numFaces);

		Faces = (Face *)malloc(sizeof(Face) * numFaces);

		for (i = 0 ; i < numFaces ; i++)
			getvar(Faces[i]);

		if (hasWeights)
		{
			char joint[64];

			getvar(numSkinJoints);
			for (i = 0 ; i < numSkinJoints ; i++)
			{
				getvar(joint);
				jointNames.append(QString(joint));
			}
		}

		numMorphs = 0;

		while (true)
		{
			char morphName[64];

			getvar(morphName);
			if (!strcmp(morphName, "End Morphs"))
				break;

			if (numMorphs == 0)
				Morphs = (Morph *)malloc(sizeof(Morph));
			else
				Morphs = (Morph *)realloc(Morphs, sizeof(Morph) * (numMorphs + 1));

			Morph *current = &Morphs[numMorphs];
			numMorphs++;

			memcpy(current->morphName, morphName, 64);
			getvar(current->numVertices);

			current->Vertices = (MorphVertex *)malloc(sizeof(MorphVertex) * current->numVertices);
			for (i = 0 ; i < current->numVertices ; i++)
				getvar(current->Vertices[i]);
		}

		getvar(numRemaps);

		if (numRemaps > 0)
		{
			Remaps = (Remap *)malloc(sizeof(Remap) * numRemaps);
			for (i = 0 ; i < numRemaps ; i++)
				getvar(Remaps[i]);
		}

		source.close();

		return true;
	}

    std::string LegacyAvatarModule::type_name_static_ = "LegacyAvatar";

    LegacyAvatarModule::LegacyAvatarModule() 
    : ModuleInterface(type_name_static_)
    {
    }
    
    // virtual
    LegacyAvatarModule::~LegacyAvatarModule()
    {

    }

    void LegacyAvatarModule::Load()
    {
		if (mesh_eye == 0)
		{
			mesh_eye = new Mesh;
			if(mesh_eye->Load("data/avatar_eye.llm"))
				LogInfo("Eye mesh loaded");
		}

		if (mesh_eyelashes == 0)
		{
			mesh_eyelashes = new Mesh;
			if(mesh_eyelashes->Load("data/avatar_eyelashes.llm"))
				LogInfo("Eyelash mesh loaded");
		}

		if (mesh_hair == 0)
		{
			mesh_hair = new Mesh;
			if(mesh_hair->Load("data/avatar_hair.llm"))
				LogInfo("Hair mesh loaded");
		}

		if (mesh_head == 0)
		{
			mesh_head = new Mesh;
			if(mesh_head->Load("data/avatar_head.llm"))
				LogInfo("Head mesh loaded");
		}

		if (mesh_lower_body == 0)
		{
			mesh_lower_body = new Mesh;
			if(mesh_lower_body->Load("data/avatar_lower_body.llm"))
				LogInfo("Lower body mesh loaded");
		}

		if (mesh_skirt == 0)
		{
			mesh_skirt = new Mesh;
			if(mesh_skirt->Load("data/avatar_skirt.llm"))
				LogInfo("Skirt mesh loaded");
		}

		if (mesh_upper_body == 0)
		{
			mesh_upper_body = new Mesh;
			if(mesh_upper_body->Load("data/avatar_upper_body.llm"))
				LogInfo("Upper body mesh loaded");
		}

        LogInfo("Module " + Name() + " loaded.");
    }

    void LegacyAvatarModule::Unload()
    {
        LogInfo("Module " + Name() + " unloaded.");
    }

    void LegacyAvatarModule::Initialize()
    {        
        LogInfo("Module " + Name() + " initialized.");
    }

    void LegacyAvatarModule::PostInitialize()
    {
    }

    void LegacyAvatarModule::Uninitialize()
    {
        LogInfo("Module " + Name() + " uninitialized.");
    }

    void LegacyAvatarModule::Update(f64 frametime)
    {
        RESETPROFILER;
    }

    bool LegacyAvatarModule::HandleEvent(
        event_category_id_t category_id,
        event_id_t event_id, 
        Foundation::EventDataInterface* data)
    {
        PROFILE(LegacyAvatarModule_HandleEvent);

        return false;
    }
}

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

using namespace LegacyAvatar;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
   POCO_EXPORT_CLASS(LegacyAvatarModule)
POCO_END_MANIFEST

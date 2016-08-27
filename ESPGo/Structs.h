#pragma once
struct demoheader
{
	char		demofilestamp[8];				// Should be HL2DEMO
	__int32		demoprotocol;					// Should be DEMO_PROTOCOL
	__int32		networkprotocol;				// Should be PROTOCOL_VERSION
	char		servername[MAX_OSPATH];			// Name of server
	char		clientname[MAX_OSPATH];			// Name of client who recorded the game
	char		mapname[MAX_OSPATH];				// Name of map
	char		gamedirectory[MAX_OSPATH];		// Name of game directory (com_gamedir)
	float		playback_time;					// Time of track
	__int32		playback_ticks;					// # of ticks in track
	__int32		playback_frames;				// # of frames in track
	__int32		signonlength;					// length of sigondata in bytes
};

struct QAngle
{
	float x, y, z;

	void Init(void)
	{
		x = y = z = 0.0f;
	}

	void Init(float _x, float _y, float _z)
	{
		x = _x;
		y = _y;
		z = _z;
	}
};

struct Vector
{
	float x, y, z;

	void Init(void)
	{
		x = y = z = 0.0f;
	}

	void Init(float _x, float _y, float _z)
	{
		x = _x;
		y = _y;
		z = _z;
	}
};

struct democmdinfo
{
	democmdinfo(void)
	{
	}

	// Does this really indicate a split screen? CSGO split screen? That would be awesome...and shitty
	struct Split
	{
		Split(void)
		{
			flags = FDEMO_NORMAL;
			viewOrigin.Init();
			viewAngles.Init();
			localViewAngles.Init();

			// Resampled origin/angles
			viewOrigin2.Init();
			viewAngles2.Init();
			localViewAngles2.Init();
		}

		Split&	operator=(const Split& src)
		{
			if (this == &src)
				return *this;

			flags = src.flags;
			viewOrigin = src.viewOrigin;
			viewAngles = src.viewAngles;
			localViewAngles = src.localViewAngles;
			viewOrigin2 = src.viewOrigin2;
			viewAngles2 = src.viewAngles2;
			localViewAngles2 = src.localViewAngles2;

			return *this;
		}

		const Vector& GetViewOrigin(void)
		{
			if (flags & FDEMO_USE_ORIGIN2)
			{
				return viewOrigin2;
			}
			return viewOrigin;
		}

		const QAngle& GetViewAngles(void)
		{
			if (flags & FDEMO_USE_ANGLES2)
			{
				return viewAngles2;
			}
			return viewAngles;
		}
		const QAngle& GetLocalViewAngles(void)
		{
			if (flags & FDEMO_USE_ANGLES2)
			{
				return localViewAngles2;
			}
			return localViewAngles;
		}

		void Reset(void)
		{
			flags = 0;
			viewOrigin2 = viewOrigin;
			viewAngles2 = viewAngles;
			localViewAngles2 = localViewAngles;
		}

		int32		flags;

		// original origin/viewangles
		Vector		viewOrigin;
		QAngle		viewAngles;
		QAngle		localViewAngles;

		// Resampled origin/viewangles
		Vector		viewOrigin2;
		QAngle		viewAngles2;
		QAngle		localViewAngles2;
	};

	void Reset(void)
	{
		for (int i = 0; i < MAX_SPLITSCREEN_CLIENTS; ++i)
		{
			u[i].Reset();
		}
	}

	Split			u[MAX_SPLITSCREEN_CLIENTS];
};

struct Prop_t
{
	Prop_t()
	{
	}

	Prop_t(SendPropType_t type)
		: m_type(type)
		, m_nNumElements(0)
	{
		// this makes all possible types init to 0's
		m_value.m_vector.Init();
	}

	void Print(int nMaxElements = 0)
	{
		if (m_nNumElements > 0)
		{
			printf(" Element: %d  ", (nMaxElements ? nMaxElements : m_nNumElements) - m_nNumElements);
		}

		switch (m_type)
		{
		case DPT_Int:
		{
			printf("%d\n", m_value.m_int);
		}
		break;
		case DPT_Float:
		{
			printf("%f\n", m_value.m_float);
		}
		break;
		case DPT_Vector:
		{
			printf("%f, %f, %f\n", m_value.m_vector.x, m_value.m_vector.y, m_value.m_vector.z);
		}
		break;
		case DPT_VectorXY:
		{
			printf("%f, %f\n", m_value.m_vector.x, m_value.m_vector.y);
		}
		break;
		case DPT_String:
		{
			printf("%s\n", m_value.m_pString);
		}
		break;
		case DPT_Array:
			break;
		case DPT_DataTable:
			break;
		case DPT_Int64:
		{
			printf("%lld\n", m_value.m_int64);
		}
		break;
		}

		if (m_nNumElements > 1)
		{
			Prop_t *pProp = this;
			pProp[1].Print(nMaxElements ? nMaxElements : m_nNumElements);
		}
	}

	SendPropType_t m_type;
	union
	{
		int m_int;
		float m_float;
		const char *m_pString;
		int64 m_int64;
		Vector m_vector;
	} m_value;
	int m_nNumElements;
};


struct FlattenedPropEntry
{
	FlattenedPropEntry(const CSVCMsg_SendTable::sendprop_t *prop, const CSVCMsg_SendTable::sendprop_t *arrayElementProp)
		: m_prop(prop)
		, m_arrayElementProp(arrayElementProp)
	{
	}
	const CSVCMsg_SendTable::sendprop_t *m_prop;
	const CSVCMsg_SendTable::sendprop_t *m_arrayElementProp;
};

struct PropEntry
{
	PropEntry(FlattenedPropEntry *pFlattenedProp, Prop_t *pPropValue)
		: m_pFlattenedProp(pFlattenedProp)
		, m_pPropValue(pPropValue)
	{
	}
	~PropEntry()
	{
		delete m_pPropValue;
	}

	FlattenedPropEntry *m_pFlattenedProp;
	Prop_t *m_pPropValue;
};

struct EntityEntry
{
	EntityEntry(int nEntity, uint32 uClass, uint32 uSerialNum)
		: m_nEntity(nEntity)
		, m_uClass(uClass)
		, m_uSerialNum(uSerialNum)
	{
	}
	~EntityEntry()
	{
		for (std::vector< PropEntry * >::iterator i = m_props.begin(); i != m_props.end(); i++)
		{
			delete *i;
		}
	}
	PropEntry *FindProp(const char *pName)
	{
		for (std::vector< PropEntry * >::iterator i = m_props.begin(); i != m_props.end(); i++)
		{
			PropEntry *pProp = *i;
			if (pProp->m_pFlattenedProp->m_prop->var_name().compare(pName) == 0)
			{
				return pProp;
			}
		}
		return NULL;
	}
	void AddOrUpdateProp(FlattenedPropEntry *pFlattenedProp, Prop_t *pPropValue)
	{
		//if ( m_uClass == 34 && pFlattenedProp->m_prop->var_name().compare( "m_vecOrigin" ) == 0 )
		//{
		//	printf("got vec origin!\n" );
		//}
		PropEntry *pProp = FindProp(pFlattenedProp->m_prop->var_name().c_str());
		if (pProp)
		{
			delete pProp->m_pPropValue;
			pProp->m_pPropValue = pPropValue;
		}
		else
		{
			pProp = new PropEntry(pFlattenedProp, pPropValue);
			m_props.push_back(pProp);
		}
	}
	int m_nEntity;
	uint32 m_uClass;
	uint32 m_uSerialNum;

	std::vector< PropEntry * > m_props;
};

struct ServerClass_t
{
	int nClassID;
	char strName[256];
	char strDTName[256];
	int nDataTable;

	std::vector< FlattenedPropEntry > flattenedProps;
};

struct ExcludeEntry
{
	ExcludeEntry(const char *pVarName, const char *pDTName, const char *pDTExcluding)
		: m_pVarName(pVarName)
		, m_pDTName(pDTName)
		, m_pDTExcluding(pDTExcluding)
	{
	}

	const char *m_pVarName;
	const char *m_pDTName;
	const char *m_pDTExcluding;
};

struct DT_CSPlayerEntity
{
	Vector vectorOrigin;
};

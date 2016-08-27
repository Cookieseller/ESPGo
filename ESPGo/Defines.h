#pragma once
typedef __int32			 	int32;
typedef unsigned __int32	uint32;
typedef __int64				int64;
typedef unsigned __int64	uint64;
typedef unsigned long		CRC32_t;

#define DEMO_HEADER_ID		"HL2DEMO"
#define DEMO_PROTOCOL		4
#define	MAX_OSPATH			260

#define FDEMO_NORMAL		0
#define FDEMO_USE_ORIGIN2	(1 << 0)
#define FDEMO_USE_ANGLES2	(1 << 1)
#define FDEMO_NOINTERP		(1 << 2)	// don't interpolate between this an last view

#define MAX_SPLITSCREEN_CLIENTS	2

#define NET_MAX_PAYLOAD			(262144 - 4)		// largest message we can send in bytes
#define DEMO_RECORD_BUFFER_SIZE	( 2 * 1024 * 1024 )	// temp buffer big enough to fit both string tables and server classes

#define ENTITY_SENTINEL		9999

#define DT_MAX_STRING_BITS			9
#define DT_MAX_STRING_BUFFERSIZE	(1<<DT_MAX_STRING_BITS)	// Maximum length of a string that can be sent.

#define NUM_NETWORKED_EHANDLE_SERIAL_NUMBER_BITS	10

// How many bits to use to encode an edict.
#define	MAX_EDICT_BITS					11					// # of bits needed to represent max edicts
// Max # of edicts in a level
#define	MAX_EDICTS						( 1 << MAX_EDICT_BITS )

#define SPROP_UNSIGNED					( 1 << 0 )	// Unsigned integer data.
#define SPROP_COORD						( 1 << 1 )	// If this is set, the float/vector is treated like a world coordinate. Note that the bit count is ignored in this case.
#define SPROP_NOSCALE					( 1 << 2 )	// For floating point, don't scale into range, just take value as is.
#define SPROP_ROUNDDOWN					( 1 << 3 )	// For floating point, limit high value to range minus one bit unit
#define SPROP_ROUNDUP					( 1 << 4 )	// For floating point, limit low value to range minus one bit unit
#define SPROP_NORMAL					( 1 << 5 )	// If this is set, the vector is treated like a normal (only valid for vectors)
#define SPROP_EXCLUDE					( 1 << 6 )	// This is an exclude prop (not excludED, but it points at another prop to be excluded).
#define SPROP_XYZE						( 1 << 7 )	// Use XYZ/Exponent encoding for vectors.
#define SPROP_INSIDEARRAY				( 1 << 8 )	// This tells us that the property is inside an array, so it shouldn't be put into the flattened property list. Its array will point at it when it needs to.
#define SPROP_PROXY_ALWAYS_YES			( 1 << 9 )	// Set for datatable props using one of the default datatable proxies like SendProxy_DataTableToDataTable that always send the data to all clients.
#define SPROP_IS_A_VECTOR_ELEM			( 1 << 10 )	// Set automatically if SPROP_VECTORELEM is used.
#define SPROP_COLLAPSIBLE				( 1 << 11 )	// Set automatically if it's a datatable with an offset of 0 that doesn't change the pointer (ie: for all automatically-chained base classes).
#define SPROP_COORD_MP					( 1 << 12 ) // Like SPROP_COORD, but special handling for multiplayer games
#define SPROP_COORD_MP_LOWPRECISION 	( 1 << 13 ) // Like SPROP_COORD, but special handling for multiplayer games where the fractional component only gets a 3 bits instead of 5
#define SPROP_COORD_MP_INTEGRAL			( 1 << 14 ) // SPROP_COORD_MP, but coordinates are rounded to integral boundaries
#define SPROP_CELL_COORD				( 1 << 15 ) // Like SPROP_COORD, but special encoding for cell coordinates that can't be negative, bit count indicate maximum value
#define SPROP_CELL_COORD_LOWPRECISION 	( 1 << 16 ) // Like SPROP_CELL_COORD, but special handling where the fractional component only gets a 3 bits instead of 5
#define SPROP_CELL_COORD_INTEGRAL		( 1 << 17 ) // SPROP_CELL_COORD, but coordinates are rounded to integral boundaries
#define SPROP_CHANGES_OFTEN				( 1 << 18 )	// this is an often changed field, moved to head of sendtable so it gets a small index
#define SPROP_VARINT					( 1 << 19 )	// use var int encoded (google protobuf style), note you want to include SPROP_UNSIGNED if needed, its more efficient


// Demo messages
enum
{
	// it's a startup message, process as fast as possible
	dem_signon = 1,
	// it's a normal network packet that we stored off
	dem_packet,
	// sync client clock to demo tick
	dem_synctick,
	// console command
	dem_consolecmd,
	// user input command
	dem_usercmd,
	// network data tables
	dem_datatables,
	// end of time.
	dem_stop,
	// a blob of binary data understood by a callback function
	dem_customdata,

	dem_stringtables,

	// Last command
	dem_lastcmd = dem_stringtables
};

// Flags for delta encoding header
enum HeaderFlags
{
	FHDR_ZERO = 0x0000,
	FHDR_LEAVEPVS = 0x0001,
	FHDR_DELETE = 0x0002,
	FHDR_ENTERPVS = 0x0004,
};

enum UpdateType
{
	EnterPVS = 0,	// Entity came back into pvs, create new entity if one doesn't exist
	LeavePVS,		// Entity left pvs
	DeltaEnt,		// There is a delta for this entity.
	PreserveEnt,	// Entity stays alive but no delta ( could be LOD, or just unchanged )
	Finished,		// finished parsing entities successfully
	Failed,			// parsing error occured while reading entities
};

enum SendPropType_t
{
	DPT_Int = 0,
	DPT_Float,
	DPT_Vector,
	DPT_VectorXY, // Only encodes the XY of a vector, ignores Z
	DPT_String,
	DPT_Array,	// An array of the base types (can't be of datatables).
	DPT_DataTable,
	DPT_Int64,
	DPT_NUMSendPropTypes
};

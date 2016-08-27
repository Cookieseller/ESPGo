#include "stdafx.h"
#include "FileParser.h"

void CFileParser::ParseDemoFile(CDemoFile &demoFile)
{
	unsigned char	cmd;
	bool			demoFinished = false;
	int32			tick = 0;

	while (!demoFinished)
	{
		demoFile.GetCommandAtPosition(cmd, tick);
		
		switch (cmd)
		{
			case dem_synctick:
				// Nothing to do here, just an inserted tick with no information
				break;
			case dem_stop:
				demoFinished = true;
				break;
			case dem_consolecmd:
				ReadConsoleCmd(demoFile);
				// ReadRawData here
				break;
			case dem_datatables:
				ReadDataTables(demoFile);
				// Parse Datatables
				break;
			case dem_stringtables:
				ReadStringTables(demoFile);
				break;
			case dem_usercmd:
				ReadUserCmd(demoFile);
				break;
			case dem_signon:
			case dem_packet:
				ReadPacket(demoFile, tick);
				break;
		}
	}
}

void CFileParser::ReadConsoleCmd(CDemoFile &demoFile)
{
	demoFile.GetRawData(NULL, 0);
}

void CFileParser::ReadDataTables(CDemoFile &demoFile)
{
	char *data = (char *)malloc(DEMO_RECORD_BUFFER_SIZE);
	CBitRead buf(data, DEMO_RECORD_BUFFER_SIZE);
	demoFile.GetRawData((char*)buf.GetBasePointer(), buf.GetNumBytesLeft());
	buf.Seek(0);
	
	netMessageDecoder.ParseDataTable(buf);
	// Do sth with the data

	free(data);
}

void CFileParser::ReadStringTables(CDemoFile &demoFile)
{
	char *data = (char *)malloc(DEMO_RECORD_BUFFER_SIZE);
	CBitRead buf(data, DEMO_RECORD_BUFFER_SIZE);
	demoFile.GetRawData((char*)buf.GetBasePointer(), buf.GetNumBytesLeft());
	buf.Seek(0);

	// Do sth with the data

	free(data);
}

int32 CFileParser::ReadUserCmd(CDemoFile &demoFile)
{
	int	dummy;
	return demoFile.GetUserCmd(NULL, dummy);
}

void CFileParser::ReadPacket(CDemoFile &demoFile, int32 tick)
{
	int			dummy;
	democmdinfo info;
	char		data[NET_MAX_PAYLOAD];

	demoFile.GetCommandInfo(info);
	demoFile.GetSequenceInfo(dummy, dummy);

	CBitRead buffer(data, NET_MAX_PAYLOAD);
	int32 length = demoFile.GetRawData((char*)buffer.GetBasePointer(), buffer.GetNumBytesLeft());
	buffer.Seek(0);

	ParseReadData(buffer, length, tick);
}

void PrintDeltaEntities(std::list<EntityEntry> list, int32 tick)
{
	for (auto it = list.begin(); it != list.end(); ++it)
	{
		printf("{id: %d, fields: [", it->m_nEntity);
		for (std::vector<PropEntry *>::iterator propIt = it->m_props.begin(); propIt != it->m_props.end(); ++propIt)
		{
			const CSVCMsg_SendTable::sendprop_t *pSendProp = (*propIt)->m_pFlattenedProp->m_prop;
			printf("%s: ", pSendProp->var_name().c_str());
			(*propIt)->m_pPropValue->Print();
			if (std::next(propIt) != it->m_props.end())
			{
				printf(", ");
			}
		}
		

		printf("]}");
	}
}

void CFileParser::ParseReadData(CBitRead buffer, int32 length, int32 tick)
{
	//printf("\n\nTick: %d", tick);
	while (buffer.GetNumBytesRead() < length)
	{
		uint32 cmd = buffer.ReadVarInt32();
		uint32 size = buffer.ReadVarInt32();

		switch (cmd)
		{
		case net_NOP:
			//printf("NOP");
			break;
		case net_Disconnect:
			//printf("Disconnect");
			break;
		case net_File:
			//printf("File");
			break;
		case net_Tick:
			//printf("Tick");
			break;
		case net_StringCmd:
			//printf("StringCmd");
			break;
		case net_SetConVar:
			//printf("SetConVar");
			break;
		case net_SignonState:
			//printf("SignonState");"
			break;
		case svc_ServerInfo:
			//printf("ServerInfo");
			break;
		case svc_SendTable:
			//printf("SendTable");
			break;
		case svc_GameEvent:
			//printf("GameEvent");
			break;
		case svc_PacketEntities:
			netMessageDecoder.DecodeNetMessage(buffer.GetBasePointer() + buffer.GetNumBytesRead(), size, entities);
			PrintDeltaEntities(entities, tick);
			//printf("PacketEntities");
			break;
		case svc_TempEntities:
			//printf("TempEntities");
			break;
		default:
			//printf("%d\n", cmd);
			break;
		}

		buffer.SeekRelative(size * 8);
	}
}


















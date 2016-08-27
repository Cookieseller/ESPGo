#pragma once
class CFileParser
{
public:
	void ParseDemoFile(CDemoFile &demoFile);
	void ReadConsoleCmd(CDemoFile &demoFile);
	void ReadDataTables(CDemoFile &demoFile);
	void ReadStringTables(CDemoFile &demoFile);
	int32 ReadUserCmd(CDemoFile &demoFile);
	void ReadPacket(CDemoFile &demoFile, int32 tick);
	void ParseReadData(CBitRead buffer, int32 length, int32 tick);

private:
	CNetMessageDecoder netMessageDecoder;
	std::list<EntityEntry> entities;
};

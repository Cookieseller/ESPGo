#pragma once
class CNetMessageDecoder
{
private:
	EntityEntry* FindEntity(int nEntity);
	int ReadFieldIndex(CBitRead &entityBitBuffer, int lastIndex, bool bNewWay);
	bool ReadNewEntity(CBitRead &entityBitBuffer, EntityEntry *pEntity);
	CSVCMsg_SendTable* GetTableByClassID(uint32 nClassID);
	FlattenedPropEntry* GetSendPropByIndex(uint32 uClass, uint32 uIndex);
	EntityEntry* CNetMessageDecoder::AddEntity(int nEntity, uint32 uClass, uint32 uSerialNum);
	void CNetMessageDecoder::RemoveEntity(int nEntity);
	bool ReadFromBuffer(CBitRead &buffer, void **pBuffer, int& size);
	void FlattenDataTable(int nServerClass);
	void GatherExcludes(CSVCMsg_SendTable *pTable);
	CSVCMsg_SendTable* GetTableByName(const char *pName);
	void GatherProps(CSVCMsg_SendTable *pTable, int nServerClass);
	bool IsPropExcluded(CSVCMsg_SendTable * pTable, const CSVCMsg_SendTable::sendprop_t & checkSendProp);
	void GatherProps_IterateProps(CSVCMsg_SendTable *pTable, int nServerClass, std::vector< FlattenedPropEntry > &flattenedProps);



public:
	CNetMessageDecoder();
	bool ParseDataTable(CBitRead &buffer);
	void DecodeNetMessage(const void *parseBuffer, int BufferSize, std::list<EntityEntry> &entities);

	bool g_bDumpPacketEntities = false;
	int s_nServerClassBits;
	std::vector< ServerClass_t > s_ServerClasses;
	std::vector< CSVCMsg_SendTable > s_DataTables;
	std::vector< ExcludeEntry > s_currentExcludes;
	std::vector< EntityEntry * > s_Entities;
};


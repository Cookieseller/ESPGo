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
	bool EntityEnterPVS(CBitRead &entityBitBuffer, int newEntity);
	bool EntityLeavePVS(int newEntity, bool isDelta);
	bool EntityDelta(CBitRead &entityBitBuffer, int newEntity);

	bool g_bDumpPacketEntities = false;
	int m_nServerClassBits;
	std::vector< ServerClass_t > m_serverClasses;
	std::vector< CSVCMsg_SendTable > m_dataTables;
	std::vector< ExcludeEntry > m_currentExcludes;
	std::vector< EntityEntry * > m_entities;

public:
	CNetMessageDecoder();
	bool ParseDataTable(CBitRead &buffer);
	void DecodeNetMessage(const void *parseBuffer, int bufferSize, std::list<EntityEntry> &entities);
};

